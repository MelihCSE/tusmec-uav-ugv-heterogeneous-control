#include "vehicle.h"
#include "net_tcp.h"
#include "mavlink_helper.h"

#include <stdint.h>
#include <unistd.h>

// ArduRover Ride Mode Constants
#define ROVER_MODE_HOLD 4
#define ROVER_MODE_GUIDED 15

static MavlinkParser_t ugv_parsers[16];

static void ugv_update_telemetry(Vehicle_t *self){
    if(self == NULL) return;

    MavlinkParser_t* parser = &ugv_parsers[self->mavlink_id % 16];
    mavlink_poll_telemetry(self, parser);
}

static int ugv_take_off(Vehicle_t* self, double altitude){
    if(self == NULL) return -1;

    // UGV does not use altitude
    (void)altitude; 

    if(mavlink_send_set_mode(self->sock_fd, self->mavlink_id, ROVER_MODE_GUIDED) < 0){
        return -1;
    }
    usleep(200000);

    if(mavlink_send_arm(self->sock_fd, self->mavlink_id, true) < 0){
        return -1;
    }

    usleep(500000);

    return mavlink_send_takeoff(self->sock_fd, self->mavlink_id, 0.0f);
}

static int ugv_landing(Vehicle_t* self){
    if(self == NULL) return -1;

    return mavlink_send_set_mode(self->sock_fd, self->mavlink_id, ROVER_MODE_HOLD);
}

static void ugv_guidance(Vehicle_t* self, double longitude, double latitude, double altitude){
    if(self == NULL) return;

    // UGV does not use altitude
    (void)altitude;

    mavlink_send_guided_target(self->sock_fd, self->mavlink_id, latitude, longitude, 0.0f);
}

static void ugv_hold_vehicle(Vehicle_t* self){
    if(self == NULL) return;

    mavlink_send_set_mode(self->sock_fd, self->mavlink_id, ROVER_MODE_HOLD);
}

Vehicle_t UGV_create(uint8_t id, uint16_t port){
    Vehicle_t ugv;
    memset(&ugv, 0, sizeof(ugv));

    snprintf(ugv.name, VEHICLE_NAME_LENGTH, "UGV_%u", id);
    ugv.mavlink_id = id;
    ugv.port = port;
    ugv.sock_fd = tcp_connect("127.0.0.1", port);
    if(ugv.sock_fd >= 0){
        mavlink_request_data_stream(ugv.sock_fd, ugv.mavlink_id, 10);
    }

    ugv.VTable.update_telemetry = ugv_update_telemetry;
    ugv.VTable.take_off = ugv_take_off;
    ugv.VTable.landing = ugv_landing;
    ugv.VTable.guidance = ugv_guidance;
    ugv.VTable.hold_vehicle = ugv_hold_vehicle;

    return ugv;
}