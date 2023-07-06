#!/bin/bash
echo "This is a shell script for generating RMA certificate"
gcc unsigned_cert_generator.c -o unsigned_certificate.o
#three parameters passed as arguments to unsigned_certificate.o are the device unique ID(12-byte) passed in 4-byte each
sudo ./unsigned_certificate.o $1 $2 $3
sleep 1
echo "unsigned certificate generated..."
openssl dgst -sha256 -binary -sign ./rsa_private.pem -out ./signed_certificate.bin ./unsigned_certificate.bin
sleep 1
echo "Signature Generated..."
openssl dgst -sha256 -verify "./rsa_public.pem"  -signature "./signed_certificate.bin" "./unsigned_certificate.bin"
echo "Signature Verified..."
gcc signed_cert_generator.c -o signed_certificate.o
sudo ./signed_certificate.o $1 $2 $3
echo "Signature appended and RMA certificate available in certificate.c file"