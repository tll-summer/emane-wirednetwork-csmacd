This model (Csmacd mac layer) is a wired network emulation module plugged into EMANE, used as a csmacd mac layer. It adopts a different architecture from wireless NEM. Instead of transmitting through the only OTA model shared by all the NEMs, packets deliver through channel model only to the NEMs whose mac layer connects the same channel. 
User guideline:
Prepare:
l	Replace emanemaclayerimpl.h, emanemaclayerimpl.inl and emanemactypes.h (in /usr/include/emane) with the new ones.
l	Add libcsmacdmaclayer.so into the /usr/lib/.

Deploying  attributes:
l	channel (int): channel id to identify which channel the mac layer is connected to.
l	prodelay (int ,bit per second): the propagating speed of the channel.
l	datarate (int, bit per second): the transmitting speed.
l	cwmax (int): the max competing window of the csmacd protocol.
l	retrylimits (int): the max retry times of the csmacd protocol.

There are three demos for you to take as examples.