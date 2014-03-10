This model (Csmacd mac layer) is a wired network emulation module plugged into EMANE, used as a csmacd mac layer. It adopts a different architecture from wireless NEM. Instead of transmitting through the only OTA model shared by all the NEMs, packets deliver through channel model only to the NEMs whose mac layer connects the same channel. 

User guideline:
Prepare:
1.Replace emanemaclayerimpl.h, emanemaclayerimpl.inl and emanemactypes.h (in /usr/include/emane) with the new ones.
2.Add libcsmacdmaclayer.so into the /usr/lib/.

Deploying  attributes:
* channel (int): channel id to identify which channel the mac layer is connected to.
* prodelay (int ,bit per second): the propagating speed of the channel.
* datarate (int, bit per second): the transmitting speed.
* cwmax (int): the max competing window of the csmacd protocol.
* retrylimits (int): the max retry times of the csmacd protocol.

There are three demos for you to take as examples.

demo2:

 (n) (n) (n) (n)
  |   |   |   |
-----------------

----:wired channel
(n):node


demo6:

(n)----(X)-----(n)

(X):router


demo7:
                 (c)
                  |
                  |
(s)-----(X)-----(AP)----(c)
                  |
                  |
                 (c)

(s):server (c):client (AP):access point 
Between (AP) and (c) is wireless channel