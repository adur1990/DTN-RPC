#include "rpc.h"

time_ms_t next_time;
MSP_SOCKET sock_msp = MSP_SOCKET_NULL;
int mdp_fd_msp;
struct mdp_sockaddr addr_msp;

// Handler of the RPC server
size_t rpc_server_msp_handler (MSP_SOCKET sock, msp_state_t state, const uint8_t *payload, size_t len, void *UNUSED(context)) {
    size_t ret = 0;

    // If there is an errer on the socket, stop it.
    if (state & (MSP_STATE_SHUTDOWN_REMOTE | MSP_STATE_CLOSED | MSP_STATE_ERROR)) {
        pwarn("Socket closed.");
        msp_stop(sock);
        ret = len;
    }

    // If we receive something, handle it.
    if (payload && len) {
        // First make sure, we received a RPC call packet.
        if (read_uint16(&payload[0]) == RPC_PKT_CALL) {
            pinfo("Received RPC via MSP.");
            // Parse the payload to the RPCProcedure struct
            struct RPCProcedure rp = rpc_server_parse_call(payload, len);

            // Check, if we offer this procedure.
            if (rpc_server_check_offered(&rp) == 0) {
                pinfo("Offering desired RPC. Sending ACK.");
                // Compile and send ACK packet.
                uint8_t ack_payload[2];
                write_uint16(&ack_payload[0], RPC_PKT_CALL_ACK);
                ret = msp_send(sock, ack_payload, sizeof(ack_payload));

                // Try to execute the procedure.
			    uint8_t result_payload[2 + 127 + 1];
                if (rpc_server_excecute(result_payload, rp) == 0) {
					pinfo("Sending result via MSP.");
        			msp_send(sock, result_payload, sizeof(result_payload));
                    pinfo("RPC execution was successful.");
                    ret = len;
                } else {
					pfatal("RPC execution was not successful. Aborting.");
                    ret = len;
                }
            } else {
                pwarn("Not offering desired RPC. Ignoring.");
                ret = len;
            }
        }
    }
    return ret;
}

// Setup the MSP part.
int rpc_server_msp_setup () {
	// Init the address struct and set the sid to a local sid and port where to listen at.
    bzero(&addr_msp, sizeof addr_msp);
    addr_msp.sid = BIND_PRIMARY;
    addr_msp.port = MDP_PORT_RPC_MSP;

    // Create MDP socket.
	if ((mdp_fd_msp = mdp_socket()) < 0) {
		pfatal("Could not create MDP listening socket for MSP. Aborting.");
		return -1;
	}

    // Sockets should not block.
    set_nonblock(mdp_fd_msp);
    set_nonblock(STDIN_FILENO);
    set_nonblock(STDERR_FILENO);
    set_nonblock(STDOUT_FILENO);

    // Create MSP socket.
    sock_msp = msp_socket(mdp_fd_msp, 0);
    // Connect to local socker ...
    msp_set_local(sock_msp, &addr_msp);
    // ... and listen it.
    msp_listen(sock_msp);

    // Set the handler to handle incoming packets.
    msp_set_handler(sock_msp, rpc_server_msp_handler, NULL);

	return 0;
}

// MSP listener.
void rpc_server_msp_listen () {
	// Process MSP socket.
	msp_processing(&next_time);
	// Receive the data from the socket.
	msp_recv(mdp_fd_msp);
}

// MSP Cleanup.
void rpc_server_msp_cleanup () {
	sock_msp = MSP_SOCKET_NULL;
	msp_close_all(mdp_fd_msp);
	mdp_close(mdp_fd_msp);
	msp_processing(&next_time);
}
