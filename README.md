
# HA-rf-switch-mercury-350.115
My way to controll the 433mHz Mercury 350.115 switch via Home Assistant on a raspberr pi

After alot of research and testing I finaly managed to get this to work.
There are alot of tutorials on how to controll RF-switches but none of them seems to work with the Mercury 350.115. I finaly found the one made by Geoff Johnson and modified by Ian Parsons which worked.

I managed to use a RF sniffer to find out the values that was sent from the remote.
For the first button I got 4543795 or in binary 10001010101010100110011 which I want to send to the power outlet. But the binary code needs to be repackage like this.

0000000000001000111010001000100011101000111010001110100011101000111010001110100010001110111010001000111011101000000000000000000000

The sequence start with 9 bits of 0.

000000000

After that a 0 is sent as 0001 and a 1 as 1101
So 10001010101010100110011
Translates into 0001000111010001000100011101000111010001110100011101000111010001110100010001110111010001000111011101

If you view this text as raw then youcan see how the next two lines corespond to each other.

0   0   1   0   0   0   1   0   1   0   1   0   1   0   1   0   1   1   1   0   0   0   0   1   1
0001000111010001000100011101000111010001110100011101000111010001110100010001110111010001000111011101

The sequence ends with 21 bits of 0

000000000000000000000

The code rely on hardcoding the binary code that is sent from the remote. I aim is to make it so that I don't need to hardcode each button so that I can send the decimal value when I call the program.
