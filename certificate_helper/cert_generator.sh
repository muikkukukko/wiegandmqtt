#!/bin/bash

BROKER_CN="192.168.0.158" # This has to match host name when connecting, either domain name or IP
CERT_C="FI"
CERT_ST="Pirkanmaa"
CERT_L="Tampere"
CERT_O="hyva10"

# Key security warning
echo ""
echo "===================================================================="
echo "!!! Script does NOT encrypt key files! For development use only! !!!"
echo "===================================================================="
echo ""

# CA
echo "Generating CA"
openssl req -new -nodes -x509 -days 1000 -extensions v3_ca -keyout ca.key -out ca.crt -subj "/C=$CERT_C/ST=$CERT_ST/L=$CERT_L/CN=superca/O=$CERT_O"

# Broker
echo "Generating broker"
openssl genrsa -out broker.key 2048
openssl req -out broker.csr -key broker.key -new -subj "/C=$CERT_C/ST=$CERT_ST/L=$CERT_L/CN=$BROKER_CN/O=$CERT_O"
openssl x509 -req -in broker.csr -CA ./ca.crt -CAkey ./ca.key -CAcreateserial -out broker.crt -days 1000

rm broker.csr

# Client
echo "Generating client"
openssl genrsa -out client.key 2048
openssl req -out client.csr -key client.key -new -subj "/C=$CERT_C/ST=$CERT_ST/L=$CERT_L/CN=iotdevice/O=$CERT_O"
openssl x509 -req -in client.csr -CA ./ca.crt -CAkey ./ca.key -CAcreateserial -out client.crt -days 1000

read -p "Do you want to copy files to Mosquitto certificate dirs? (y/n) " yn

case $yn in
	[yY] )  echo Copying...
		sudo cp ./ca.crt /etc/mosquitto/ca_certificates
		sudo cp ./broker.crt /etc/mosquitto/certs
		sudo cp ./broker.key /etc/mosquitto/certs
		sudo chmod a+r /etc/mosquitto/certs/broker.key
		;;
	[nN] ) echo exiting...;
		exit;;
	* ) echo invalid response;;
esac

