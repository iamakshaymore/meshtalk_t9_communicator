#include "LoRaHelper.h"
#include "DataStore.h"
#include "configuration.h"
#include "NodeDB.h"
#include "mesh/MeshService.h"
#include "mesh/MeshTypes.h"
#include "mesh/Channels.h"
#include "mesh/Router.h"
#include "mesh/generated/meshtastic/mesh.pb.h"
#include "gps/RTC.h" // for getTime() function
#include <algorithm>

// External references with safe access patterns
extern meshtastic_DeviceState devicestate;
extern MeshService *service;
extern Router *router;
extern Channels channels;

// Static member initialization
String LoRaHelper::lastLongName = "";
String LoRaHelper::lastShortName = "";
bool LoRaHelper::initialized = false;

void LoRaHelper::init() {
    initialized = true;
}

String LoRaHelper::getDeviceLongName() {
    if (!initialized) {
        init();
    }
    
    // Get device name from global owner variable
    if (strlen(owner.long_name) > 0) {
        return String(owner.long_name);
    }
    
    // Fallback to default name
    return "Meshtastic";
}

String LoRaHelper::getDeviceShortName() {
    if (!initialized) {
        init();
    }
    
    // Get device short name from global owner variable
    if (strlen(owner.short_name) > 0) {
        return String(owner.short_name);
    }
    
    // Fallback to default
    return "MT";
}

bool LoRaHelper::hasChanged() {
    String currentLong = getDeviceLongName();
    String currentShort = getDeviceShortName();
    
    bool changed = (currentLong != lastLongName) || (currentShort != lastShortName);
    
    if (changed) {
        lastLongName = currentLong;
        lastShortName = currentShort;
    }
    
    return changed;
}

int LoRaHelper::getRSSI() {
    // Get RSSI from radio interface if available
    // This would require access to the radio interface
    return 0; // TODO: Implement RSSI reading
}

int LoRaHelper::getNodeCount() {
    auto nodedbp = nodeDB;
    if (nodedbp) {
        return nodedbp->getNumOnlineMeshNodes();
    }
    return 0;
}

bool LoRaHelper::isLoRaOnline() {
    // Check if mesh service is running
    return service != nullptr;
}

std::vector<NodeInfo> LoRaHelper::getNodesList(int maxNodes, bool includeOffline) {
    std::vector<NodeInfo> nodes;
    
    if (!nodeDB) {
        return nodes; // Return empty list if nodeDB not available
    }
    
    // Get all mesh nodes
    size_t totalNodes = nodeDB->getNumMeshNodes();
    
    for (size_t i = 0; i < totalNodes && nodes.size() < maxNodes; i++) {
        auto meshNode = nodeDB->getMeshNodeByIndex(i);
        if (!meshNode || !nodeInfoLiteHasUser(meshNode)) {
            continue; // Skip nodes without user info
        }
        
        // Skip our own node
        if (meshNode->num == nodeDB->getNodeNum()) {
            continue;
        }
        
        // Check if node is online
        bool online = isNodeOnline(meshNode->last_heard);
        if (!includeOffline && !online) {
            continue;
        }
        
        NodeInfo nodeInfo;
        nodeInfo.nodeNum = meshNode->num;
        
        // Copy strings to fixed char arrays (no dynamic allocation)
        strncpy(nodeInfo.longName, meshNode->long_name, sizeof(nodeInfo.longName) - 1);
        nodeInfo.longName[sizeof(nodeInfo.longName) - 1] = '\0';
        
        strncpy(nodeInfo.shortName, meshNode->short_name, sizeof(nodeInfo.shortName) - 1);
        nodeInfo.shortName[sizeof(nodeInfo.shortName) - 1] = '\0';
        
        nodeInfo.lastHeard = meshNode->last_heard;
        nodeInfo.snr = meshNode->snr;
        nodeInfo.signalBars = snrToSignalBars(meshNode->snr);
        nodeInfo.isOnline = online;
        nodeInfo.isFavorite = nodeInfoLiteIsFavorite(meshNode);
        nodeInfo.viaInternet = nodeInfoLiteViaMqtt(meshNode);
        nodeInfo.hopsAway = meshNode->has_hops_away ? meshNode->hops_away : 0;
        
        // Use node number as fallback if no long name
        if (strlen(nodeInfo.longName) == 0) {
            snprintf(nodeInfo.longName, sizeof(nodeInfo.longName), "Node %08X", meshNode->num);
        }
        
        // Use first two characters as short name if empty
        if (strlen(nodeInfo.shortName) == 0) {
            if (strlen(nodeInfo.longName) >= 2) {
                strncpy(nodeInfo.shortName, nodeInfo.longName, 2);
                nodeInfo.shortName[2] = '\0';
            } else {
                snprintf(nodeInfo.shortName, sizeof(nodeInfo.shortName), "%02X", meshNode->num & 0xFF);
            }
        }
        
        nodes.push_back(nodeInfo);
    }
    
    // Sort nodes by last heard (most recent first), then by favorites
    std::sort(nodes.begin(), nodes.end(), [](const NodeInfo& a, const NodeInfo& b) {
        // Favorites first
        if (a.isFavorite && !b.isFavorite) return true;
        if (!a.isFavorite && b.isFavorite) return false;
        
        // Online nodes before offline
        if (a.isOnline && !b.isOnline) return true;
        if (!a.isOnline && b.isOnline) return false;
        
        // Then by most recently heard
        return a.lastHeard > b.lastHeard;
    });
    
    return nodes;
}

int LoRaHelper::snrToSignalBars(float snr) {
    // Convert SNR to signal bars (0-4)
    // SNR values: excellent > 10dB, good > 5dB, fair > 0dB, poor > -10dB, very poor <= -10dB
    if (snr >= 10.0f) return 4;        // Excellent
    else if (snr >= 5.0f) return 3;    // Good
    else if (snr >= 0.0f) return 2;    // Fair
    else if (snr >= -10.0f) return 1;  // Poor
    else return 0;                     // Very poor
}

bool LoRaHelper::isNodeOnline(uint32_t lastHeard) {
    if (lastHeard == 0) return false;
    
    uint32_t now = getTime();
    uint32_t elapsed = now - lastHeard;
    
    // Consider online if heard within last 2 hours (7200 seconds)
    return elapsed < 7200;
}

MessageInfo LoRaHelper::getLastReceivedMessage() {
    // Try to get the latest message from DataStore first
    MessageInfo storeMessage = DataStore::getInstance().getLatestMessage();
    if (storeMessage.isValid) {
        return storeMessage;
    }
    
    // Fallback to device state if no stored messages
    MessageInfo info;
    
    // Check if devicestate has a valid text message
    if (!devicestate.has_rx_text_message || 
        devicestate.rx_text_message.decoded.portnum != meshtastic_PortNum_TEXT_MESSAGE_APP ||
        devicestate.rx_text_message.decoded.payload.size == 0) {
        return info; // Returns invalid message
    }
    
    const auto& packet = devicestate.rx_text_message;
    
    // Extract message text
    size_t textLen = std::min((size_t)packet.decoded.payload.size, sizeof(info.text) - 1);
    memcpy(info.text, packet.decoded.payload.bytes, textLen);
    info.text[textLen] = '\0';
    
    // Set message properties
    info.timestamp = packet.rx_time;
    info.senderNodeId = packet.from;
    info.toNodeId = packet.to;
    info.channelIndex = packet.channel;
    info.isOutgoing = (packet.from == 0 || (nodeDB && packet.from == nodeDB->getNodeNum()));
    
    // Determine if this is a direct message
    // Direct message: packet.to is our node ID (not broadcast)
    info.isDirectMessage = (nodeDB && packet.to == nodeDB->getNodeNum() && packet.to != NODENUM_BROADCAST);
    
    // Format channel name for channel messages
    if (!info.isDirectMessage) {
        if (info.channelIndex == 0) {
            strncpy(info.channelName, "Primary", sizeof(info.channelName) - 1);
        } else {
            snprintf(info.channelName, sizeof(info.channelName), "CH%d", info.channelIndex);
        }
    } else {
        strcpy(info.channelName, "DM");
    }
    info.channelName[sizeof(info.channelName) - 1] = '\0';
    
    info.isValid = true;
    
    // Format sender name
    String senderName = formatSenderName(info.senderNodeId, info.isOutgoing);
    strncpy(info.senderName, senderName.c_str(), sizeof(info.senderName) - 1);
    info.senderName[sizeof(info.senderName) - 1] = '\0';
    
    return info;
}

std::vector<MessageInfo> LoRaHelper::getRecentMessages(int maxMessages) {
    // Get messages from DataStore
    std::vector<MessageInfo> messages = DataStore::getInstance().getRecentMessages(maxMessages);
        
    return messages;
}

String LoRaHelper::formatSenderName(uint32_t nodeId, bool isOutgoing) {
    if (isOutgoing) {
        return "You";
    }
    
    // Look up node in NodeDB
    if (nodeDB) {
        const auto* node = nodeDB->getMeshNode(nodeId);
        if (node && nodeInfoLiteHasUser(node)) {
            // Try long name first, fallback to short name
            if (strlen(node->long_name) > 0) {
                return String(node->long_name);
            } else if (strlen(node->short_name) > 0) {
                return String(node->short_name);
            }
        }
    }
    
    // Fallback to node ID
    return String("Node !") + String(nodeId & 0xFF, HEX);
}

String LoRaHelper::formatTimeAgo(uint32_t timestamp) {
    if (timestamp == 0) {
        return "Unknown";
    }
    
    // Get current time
    uint32_t currentTime = getTime();
    if (currentTime == 0) {
        // If no valid RTC time, use millis as approximation
        currentTime = millis() / 1000;
    }
    
    if (timestamp > currentTime) {
        // Future timestamp, probably invalid
        return "Unknown";
    }
    
    uint32_t secondsAgo = currentTime - timestamp;
    
    if (secondsAgo < 60) {
        return String(secondsAgo) + "s ago";
    } else if (secondsAgo < 3600) {
        return String(secondsAgo / 60) + "m ago";
    } else if (secondsAgo < 86400) {
        return String(secondsAgo / 3600) + "h ago";
    } else {
        return String(secondsAgo / 86400) + "d ago";
    }
}

// bool LoRaHelper::sendMessage(const String& messageText, uint32_t toNodeId, uint8_t channelIndex) {
//    return sendMessage(messageText, toNodeId, channelIndex) != 0;
//}

uint32_t LoRaHelper::sendMessage(const String& messageText, uint32_t toNodeId, uint8_t channelIndex) {
    if (messageText.length() == 0) {
        return 0; // Cannot send empty message
    }
    
    if (!service || !router) {
        return 0; // MeshService or Router not available
    }
    
    // Create text message packet using Router
    auto packet = router->allocForSending();
    if (!packet) {
        return 0; // Failed to allocate packet
    }
    
    // Capture packet ID before we lose ownership
    uint32_t packetId = packet->id;

    // Set up the packet for TEXT_MESSAGE_APP
    packet->decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
    packet->to = toNodeId;
    packet->channel = channelIndex;
    packet->want_ack = true; // Request ACK to track delivery
    
    // Set message content
    const char* messageStr = messageText.c_str();
    size_t messageLen = strlen(messageStr);
    
    if (messageLen > sizeof(packet->decoded.payload.bytes)) {
        messageLen = sizeof(packet->decoded.payload.bytes);
    }
    
    packet->decoded.payload.size = messageLen;
    memcpy(packet->decoded.payload.bytes, messageStr, messageLen);
    
    // Send via mesh service
    service->sendToMesh(packet);

    // Save sent message to DataStore
    MessageInfo sentMsg;
    
    // Copy message text
    size_t textLen = std::min((size_t)messageText.length(), sizeof(sentMsg.text) - 1);
    memcpy(sentMsg.text, messageText.c_str(), textLen);
    sentMsg.text[textLen] = '\0';
    
    // Set Sender Name (Stores Recipient Name for Outgoing DMs for display purposes)
    bool isDirectMsg = (toNodeId != UINT32_MAX && toNodeId != NODENUM_BROADCAST);
    
    if (isDirectMsg && nodeDB) {
         const auto* node = nodeDB->getMeshNode(toNodeId);
         if (node && nodeInfoLiteHasUser(node) && strlen(node->long_name) > 0) {
             strncpy(sentMsg.senderName, node->long_name, sizeof(sentMsg.senderName) - 1);
         } else if (node && nodeInfoLiteHasUser(node) && strlen(node->short_name) > 0) {
              strncpy(sentMsg.senderName, node->short_name, sizeof(sentMsg.senderName) - 1);
         } else {
              // Fallback to hex ID of recipient
              snprintf(sentMsg.senderName, sizeof(sentMsg.senderName), "!%08X", toNodeId);
         }
    } else {
        strcpy(sentMsg.senderName, "Me"); 
    }
    sentMsg.senderName[sizeof(sentMsg.senderName) - 1] = '\0';
    
    // Set properties
    uint32_t now = getTime();
    if (now == 0) now = millis() / 1000;
    sentMsg.timestamp = now;

    if (nodeDB) {
        sentMsg.senderNodeId = nodeDB->getNodeNum();
    } else {
        sentMsg.senderNodeId = 0;
    }
    sentMsg.toNodeId = toNodeId;
    sentMsg.messageId = packetId;
    sentMsg.channelIndex = channelIndex;
    sentMsg.isOutgoing = true;
    sentMsg.isDirectMessage = (toNodeId != UINT32_MAX && toNodeId != NODENUM_BROADCAST);
    sentMsg.ackReceived = false;
    sentMsg.isValid = true;
    
    // Set channel name
    if (sentMsg.isDirectMessage) {
        strcpy(sentMsg.channelName, "DM");
    } else if (sentMsg.channelIndex == 0) {
        strcpy(sentMsg.channelName, "Primary");
    } else {
        snprintf(sentMsg.channelName, sizeof(sentMsg.channelName), "CH%d", sentMsg.channelIndex);
    }
    
    // Add to DataStore
    DataStore::getInstance().addMessage(sentMsg);
    
    return packetId;
}

std::vector<ChannelHelperInfo> LoRaHelper::getChannelList() {
    std::vector<ChannelHelperInfo> list;
    if (!initialized) init();

    // Iterate through all possible channels
    for (int i = 0; i < MAX_NUM_CHANNELS; i++) {
        meshtastic_Channel& ch = channels.getByIndex(i);
        
        // Only include active channels
        if (ch.role != meshtastic_Channel_Role_DISABLED) {
             ChannelHelperInfo info;
             info.index = i;
             
             // Get channel name
             const char* name = channels.getName(i);
             if (name) {
                 strncpy(info.name, name, sizeof(info.name) - 1);
                 info.name[sizeof(info.name) - 1] = '\0';
             } else {
                 snprintf(info.name, sizeof(info.name), "Ch %d", i);
             }
             
             info.isPrimary = (i == channels.getPrimaryIndex());
             info.isSecondary = (ch.role == meshtastic_Channel_Role_SECONDARY); 
             
             list.push_back(info);
        }
    }
    return list;
}