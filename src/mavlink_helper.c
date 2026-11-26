#include "mavlink_helper.h"
#include "net_tcp.h"

#define GCS_SYSID 255   // QGroundController ID
#define GCS_COMPID 190  // Mission planner ID


// Common Helper 1: Changes any mavlink message to byte and send from TCP
static int send_mavlink_msg(int sock_fd, const mavlink_message_t *msg) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
    return (tcp_send(sock_fd, buf, len) > 0) ? 0 : -1;
}

// Common helper 2: Packages all COMMAND_LONG commands (Arm, Mod, Takeoff) in one place
static int send_command_long(int sock_fd, uint8_t target_sysid, uint16_t command,
                             float p1, float p2, float p3, float p4,
                             float p5, float p6, float p7) {
    mavlink_message_t msg;
    mavlink_msg_command_long_pack(GCS_SYSID, GCS_COMPID, &msg,
                                  target_sysid, 1, command, 0,
                                  p1, p2, p3, p4, p5, p6, p7);
    return send_mavlink_msg(sock_fd, &msg);
}


void mavlink_poll_telemetry(Vehicle_t* vehicle, MavlinkParser_t* parser){

    uint8_t temp[512];
    ssize_t bytes_read = tcp_receive(vehicle->sock_fd, temp, sizeof(temp));
    if(bytes_read <= 0) return;

    for(ssize_t i=0; i<bytes_read; i++){
        if(mavlink_parse_char(vehicle->mavlink_id, temp[i], &parser->msg, &parser->status)){
            switch (parser->msg.msgid)
            {
            case MAVLINK_MSG_ID_HEARTBEAT: {
                mavlink_heartbeat_t hb;
                mavlink_msg_heartbeat_decode(&parser->msg, &hb);
                vehicle->telemetry.isArmed = (hb.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) != 0;
                vehicle->telemetry.autopilot = (uint8_t)hb.custom_mode;

                break;
            }
            
            case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
                mavlink_global_position_int_t pos;
                mavlink_msg_global_position_int_decode(&parser->msg, &pos);
                vehicle->telemetry.altitude = (double)pos.relative_alt / 1000.0;
                vehicle->telemetry.latitude = (double)pos.lat / 1e7;
                vehicle->telemetry.longitude = (double)pos.lon / 1e7;
                vehicle->telemetry.gps_flag = true;
                
                break;
            }
            default:
                break;
            }
        }
    }
}

int mavlink_request_data_stream(int sock_fd, uint8_t target_sysid, uint16_t rate_hz){
    mavlink_message_t msg;
    mavlink_msg_request_data_stream_pack(GCS_SYSID, GCS_COMPID, &msg, target_sysid, 1, MAV_DATA_STREAM_ALL, rate_hz, 1);

    return send_mavlink_msg(sock_fd, &msg);
}

int mavlink_send_arm(int sock_fd, uint8_t target_sysid, bool arm){
    return send_command_long(sock_fd, target_sysid, MAV_CMD_COMPONENT_ARM_DISARM, 
                            arm ? 1.0f : 0.0f, 0, 0, 0, 0, 0, 0);

}

int mavlink_send_set_mode(int sock_fd, uint8_t target_sysid, uint32_t custom_mode){
    return send_command_long(sock_fd, target_sysid, MAV_CMD_DO_SET_MODE, (float)MAV_MODE_FLAG_CUSTOM_MODE_ENABLED, 
                            (float)custom_mode, 0, 0, 0, 0, 0);
}

int mavlink_send_takeoff(int sock_fd, uint8_t target_sysid, double altitude){
    return send_command_long(sock_fd, target_sysid, MAV_CMD_NAV_TAKEOFF, 
                            0, 0, 0, 0, 0, 0, altitude);
}

int mavlink_send_guided_target(int sock_fd, uint8_t target_sysid, double lat, double lon, double alt){
    mavlink_message_t msg;
    uint16_t type_mask = 0x0DF8; // Follow only x, y, z location

    mavlink_msg_set_position_target_global_int_pack(GCS_SYSID, GCS_COMPID, &msg, 0, target_sysid, 1, MAV_FRAME_GLOBAL_RELATIVE_ALT_INT, type_mask, 
                                            (int32_t)(lat * 1e7), (int32_t)(lon * 1e7), alt, 
                                            0.0f, 0.0f, 0.0f, // vx, vy, vz
                                            0.0f, 0.0f, 0.0f, // afx, afy, afz
                                            0.0f, 0.0f);      // yaw, yaw_rate
    return send_mavlink_msg(sock_fd, &msg);
}
