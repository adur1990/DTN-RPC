#include "rpc.h"

// Function to check, if a RPC is offered by this server.
// Load and parse the rpc.conf file.
int rpc_server_check_offered (struct RPCProcedure *rp) {
    pinfo("Checking, if \"%s\" is offered.", rp->name);
    // Build the path to the rpc.conf file and open it.
    static char path[strlen(SYSCONFDIR) + strlen(SERVAL_FOLDER) + strlen(RPC_CONF_FILENAME) + 1] = "";
    FORMF_SERVAL_ETC_PATH(path, RPC_CONF_FILENAME);
    FILE *conf_file = fopen(path, "r");

    char *line = NULL;
    size_t len = 0;
    int ret = -1;

    // Read the file line by line.
    while (getline(&line, &len, conf_file) != -1) {
        // Split the line at the first space to get the return type.
        char *name = strtok(line, " ");
        // If the name matches with the received name ...
        if (strncmp(name, rp->name, strlen(name)) == 0) {
            ret = 1;
        }

        // Split the line at the second space to get the paramc.
        char *paramc = strtok(NULL, " ");
		// ... and the parameter count, the server offers this RPC.
        if (ret == 1 && strncmp(paramc, rp->paramc.paramc_s, strlen(paramc)) == 0) {
            ret = 0;
			break;
        }
    }

    // Cleanup.
    if (line) {
        free(line);
    }
    fclose(conf_file);

    return ret;
}

// Function to parse the received payload.
struct RPCProcedure rpc_server_parse_call (const uint8_t *payload, size_t len) {
    pinfo("Parsing call.");
    // Create a new rp struct.
    struct RPCProcedure rp;

    // Parse the parameter count.
    rp.paramc.paramc_n = read_uint16(&payload[2]);
	rp.paramc.paramc_s = calloc(sizeof(char), 6);
	sprintf(rp.paramc.paramc_s, "%u", read_uint16(&payload[2]));

    // Cast the payload starting at byte 5 to string.
    // The first 4 bytes are for packet type and param count.
    char ch_payload[len - 4];
    memcpy(ch_payload, &payload[4], len - 4);

    // Split the payload at the first '|'
    char *tok = strtok(ch_payload, "|");

    // Set the name of the procedure.
    rp.name = calloc(strlen(tok), sizeof(char));
    strncpy(rp.name, tok, strlen(tok));

    // Allocate memory for the parameters and split the remaining payload at '|'
    // until it's it fully consumed. Store the parameters as strings in the designated struct field.
    int i = 0;

    rp.params = calloc(rp.paramc.paramc_n, sizeof(char*));
    tok = strtok(NULL, "|");
    while (tok) {
        rp.params[i] = calloc(strlen(tok), sizeof(char));
        strncpy(rp.params[i++], tok, strlen(tok));
        tok = strtok(NULL, "|");
    }
    return rp;
}

// Execute the procedure
int rpc_server_excecute (uint8_t *result_payload, struct RPCProcedure rp) {
    pinfo("Executing \"%s\".", rp.name);
    FILE *pipe_fp;

    // Compile the rpc name and the path of the binary to one string.
    char bin[strlen(SYSCONFDIR) + strlen(BIN_FOLDER) + strlen(rp.name)];
    sprintf(bin, "%s%s%s", SYSCONFDIR, BIN_FOLDER, rp.name);

    // Since we use popen, which expects a string where the binary with all parameters delimited by spaces is stored,
    // we have to compile the bin with all parameters from the struct.
    char *flat_params = _rpc_flatten_params(rp.paramc.paramc_n, (const char **) rp.params, " ");

    char cmd[strlen(bin) + strlen(flat_params)];
    sprintf(cmd, "%s%s", bin, flat_params);

    // Open the pipe.
    if ((pipe_fp = popen(cmd, "r")) == NULL) {
        pfatal("Could not open the pipe. Aborting.");
        return -1;
    }

    // Payload. Two bytes for packet type, 126 bytes for the result and 1 byte for '\0' to make sure the result will be a zero terminated string.
    write_uint16(&result_payload[0], RPC_PKT_CALL_RESPONSE);

    // If the pipe is open ...
    if (pipe_fp) {
        // ... read the result, store it in the payload ...
        fgets((char *)&result_payload[2], 127, pipe_fp);
        memcpy(&result_payload[129], "\0", 1);
        // ... and close the pipe.
        int ret_code = pclose(pipe_fp);
        if (WEXITSTATUS(ret_code) != 0) {
            pfatal("Execution of \"%s\" went wrong. See errormessages above for more information. Status %i.", flat_params, WEXITSTATUS(ret_code));
            return -1;
        }
        pinfo("Returned result from Binary.");
    } else {
        return -1;
    }

    return 0;
}

// Main listening function.
int rpc_listen () {
	// Setup MDP and MSP.
    if (rpc_server_msp_setup() == -1) {
		pfatal("Could not setup MSP listener. Aborting.");
		return -1;
	}
	if (rpc_server_mdp_setup() == -1) {
		pfatal("Could not setup MDP listener. Aborting.");
		return -1;
	}

    // Run RPC server.
    while(running < 2){
        if (running == 1) {
			// Clean everythin up.
			rpc_server_msp_cleanup();
			rpc_server_mdp_cleanup();
            break;
        }

		// Listen to the three main parts.
		rpc_server_msp_listen();
		rpc_server_mdp_listen();
        if (rpc_server_rhizome_listen() == -1) {
			pfatal("Rhizome listening failed. Aborting.");
			running = 1;
		}

        // To not drive the CPU crazy, check only once a second for new packets.
        sleep(1);
    }
	return 0;
}
