#/bin/bash
if [[ $1 ]]
then
    cd libprotoident-2.0.8/tools/protoident/ 
    sudo ./lpi_protoident $1
else
    echo "Give the name of the snifed interface"
fi
