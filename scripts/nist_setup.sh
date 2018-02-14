#!/bin/bash
export ACV_URI_PREFIX=acvp/
export ACV_CA_FILE=certs/acvp.nist.gov.crt
export ACV_KEY_FILE=<procure from nist>
export ACV_CERT_FILE=<procure from nist>
export ACV_PORT=443
export ACV_SERVER=demo.acvts.nist.gov

# if building on Linux
export LD_LIBRARY_PATH=<path to transport lib>:<path to acvp library>

# if building on a Mac
# export LIBRARY_PATH=<path to transport lib>:<path to acvp library>



