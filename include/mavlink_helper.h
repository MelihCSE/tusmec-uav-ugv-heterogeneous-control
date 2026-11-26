#ifndef MAVLINK_HELPER_H
#define MAVLINK_HELPER_H

#include "mavlink/c_library_v2-master/ardupilotmega/mavlink.h"
#include "vehicle.h"

typedef struct 
{
    mavlink_message_t msg;
    mavlink_status_t status;

} MavlinkParser_t;

int mavlink_request_data_stream(int sock_fd, uint8_t target_sysid, uint16_t rate_hz);

void mavlink_poll_telemetry(Vehicle_t* vehicle, MavlinkParser_t* parser);

int mavlink_send_arm(int sock_fd, uint8_t target_sysid, bool arm);

int mavlink_send_set_mode(int sock_fd, uint8_t target_sysid, uint32_t custom_mode);

int mavlink_send_takeoff(int sock_fd, uint8_t target_sysid, double altitude);

int mavlink_send_guided_target(int sock_fd, uint8_t target_sysid, double lat, double lon, double alt);

#endif /* MAVLINK_HELPER_H */