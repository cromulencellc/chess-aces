#include "platform/platform.h"
#include "platform/prot.h"
#include "platform/stdlib.h"

#include "acd2.h"
#include "acd2_callbacks.h"
#include "acd2_store.h"
#include "acd2_stream.h"
#include "challenge.h"
#include "debug.h"


struct bus_proto_state BUS_PRIV_PROTO_STATE;
struct bus_proto_state BUS_MAINT_PROTO_STATE;


void main () {
    debug_string("[acd2] Initialization Begin");
    debug_uint32(PRIV_BUS);
    debug_uint32(MAINT_BUS);

    /* Initialize bus protocol state machines */
    bus_proto_state_initialize(&BUS_PRIV_PROTO_STATE, ACD_ADDRESS, &bus_callbacks);
    bus_proto_state_initialize(&BUS_MAINT_PROTO_STATE, ACD_ADDRESS, &bus_callbacks);

    set_interrupt_handler(&acd2_interrupt_handler);
    acd2_reset_timer_interrupt();

    uint32_t mie = csr_mie_get();
    mie |= R5_CSR_MIE_MTIE;
    csr_mie_set(mie);

    uint32_t mstatus = csr_mstatus_get();
    mstatus |= R5_CSR_MSTATUS_MIE;
    csr_mstatus_set(mstatus);

    debug_string("[acd2] Initialization Complete");

    wait_for_interrupt();
}


int acd2_process_request(
    struct bus_proto_state * bus_proto_state,
    uint8_t peer_id,
    const struct access_token_message * msg
) {
    struct access_token_message outgoing;

    debug_string("[acd2] processing request");

    if (msg->type == ACCESS_TOKEN_CREATE_REQUEST) {
        debug_string("[acd2] ACCESS_TOKEN_CREATE_REQUEST");

        /* Is this a valid key? */
        uint8_t permissions = get_permissions_for_key(msg->create_request.key);
        if (    (permissions & msg->create_request.access_type)
             != msg->create_request.access_type) {
                
            debug_string("Looks like the key is invalid");

            /* This key is invalid */
            outgoing.type = ACCESS_TOKEN_CREATE_RESPONSE;
            outgoing.create_response.status = 0;
            stream_set_data_outgoing(
                peer_id,
                &outgoing,
                sizeof(struct access_token_message)
            );
            // TODO: What if this fails?
            int error = bus_proto_initiate_stream(
                bus_proto_state,
                peer_id,
                stream_get_data_outgoing(peer_id),
                sizeof(struct access_token_message)
            );
            if (error) {
                debug_string("Failed to initialize stream back to client");
            }
        }
        else {
            debug_string("Looks like the key is valid");

            /* This key is valid */
            outgoing.type = ACCESS_TOKEN_CREATE_RESPONSE;
            outgoing.create_response.status = permissions;
            generate_token(outgoing.create_response.token);
            set_token(outgoing.create_response.token, permissions);
            stream_set_data_outgoing(
                peer_id,
                &outgoing,
                sizeof(struct access_token_message)
            );

            debug_string("Initiating stream to peer");
            debug_uint8(peer_id);

            int error = bus_proto_initiate_stream(
                bus_proto_state,
                peer_id,
                stream_get_data_outgoing(peer_id),
                sizeof(struct access_token_message)
            );
            if (error) {
                debug_string("Failed to initialize stream back to client");
            }
        }
    }
    else if (msg->type == ACCESS_TOKEN_VERIFICATION_REQUEST) {
        debug_string("[acd2] ACCESS_TOKEN_VERIFICATION_REQUEST");
        uint8_t permissions = get_token(msg->verification_request.token);
        outgoing.type = ACCESS_TOKEN_VERIFICATION_RESPONSE;
        outgoing.verification_response.access_type = permissions;
        memcpy(
            outgoing.verification_response.token,
            msg->verification_request.token,
            ACCESS_TOKEN_KEY_SIZE
        );
        stream_set_data_outgoing(
            peer_id,
            &outgoing,
            sizeof(struct access_token_message)
        );
        int error = bus_proto_initiate_stream(
            bus_proto_state,
            peer_id,
            stream_get_data_outgoing(peer_id),
            sizeof(struct access_token_message)
        );
        if (error) {
            debug_string("Failed to initialize stream back to client");
        }
    }
    else {
        debug_string("[acd2] Invalid request type for acd");
        debug_uint32(msg->type);
        return -1;
    }

    return 0;
}


/* Handle incoming bus traffic */
void acd2_run_bus() {
    current_bus = (uint8_t *) PRIV_BUS;
    current_bus_proto_state = &BUS_PRIV_PROTO_STATE;
    if (bus_proto_flush_blocking_send_data(&BUS_PRIV_PROTO_STATE) == 0) {
        if (READ_DATA_READY(PRIV_BUS)) {
            // debug_string("[acd2] read data is ready on priv bus");

            bus_proto_process(
                &BUS_PRIV_PROTO_STATE,
                (void *) DEVICE_READ_BUFFER(PRIV_BUS),
                *DEVICE_READ_SIZE(PRIV_BUS)
            );

            READ_DATA_COMPLETE_SET(PRIV_BUS, 1)
            READ_DATA_READY_SET(PRIV_BUS, 0)
        }
        else {
            bus_proto_process(&BUS_PRIV_PROTO_STATE, NULL, 0);
        }
    }
    else {
        debug_string("[acd2] bus_proto_flush_blocking_send_data returned non-zero");
    }

    current_bus = (uint8_t *) MAINT_BUS;
    current_bus_proto_state = &BUS_MAINT_PROTO_STATE;
    if (bus_proto_flush_blocking_send_data(&BUS_MAINT_PROTO_STATE) == 0) {
        if (READ_DATA_READY(MAINT_BUS)) {
            // debug_string("[acd2] read data is ready on maint bus. First two bytes:");
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[0]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[1]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[2]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[3]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[4]);
            // debug_uint8(DEVICE_READ_BUFFER(MAINT_BUS)[5]);
            // debug_uint32(*DEVICE_READ_SIZE(MAINT_BUS));

            bus_proto_process(
                &BUS_MAINT_PROTO_STATE,
                (void *) DEVICE_READ_BUFFER(MAINT_BUS),
                *DEVICE_READ_SIZE(MAINT_BUS)
            );

            READ_DATA_COMPLETE_SET(MAINT_BUS, 1)
            READ_DATA_READY_SET(MAINT_BUS, 0)
        }
        else {
            bus_proto_process(&BUS_MAINT_PROTO_STATE, NULL, 0);
        }
    }
    else {
        debug_string("[acd2] bus_proto_flush_blocking_send_data returned non-zero");
    }
}


void acd2_interrupt_handler() {
    uint32_t mcause = csr_mcause_get();

    if (mcause == R5_CSR_MCAUSE_MACHINE_TIMER_INTERRUPT) {
        acd2_run_bus();
        acd2_reset_timer_interrupt();
    }
}

void acd2_reset_timer_interrupt() {
    uint64_t new_interrupt_time = *MTIME_PTR + (10000 / TICK_MICROSECOND_SCALE);
    *MTIMECMP_PTR = new_interrupt_time;
}