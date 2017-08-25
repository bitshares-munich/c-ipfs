# c-ipfs
IPFS implementation in C, (not just an API client library).<br>
<br>
getting started: https://github.com/ipfs/specs/blob/master/overviews/implement-ipfs.md <br>
specifications: https://github.com/ipfs/specs <br>
getting started: https://github.com/ipfs/community/issues/177 <br> 
libp2p: https://github.com/libp2p/specs <br>
<br>
Prerequisites: To compile the C version you will need:<br>
lmdb https://github.com/jmjatlanta/lmdb<br>
c-protobuf https://github.com/kenCode-de/c-protobuf<br>
c-multihash https://github.com/kenCode-de/c-multihash<br>
c-multiaddr https://github.com/kenCode-de/c-multiaddr<br>
c-libp2p https://github.com/kenCode-de/c-libp2p<br>
<br>
And of course this project at https://github.com/kenCode-de/c-ipfs<br>
<br>
The compilation at this point is simple, but not very flexible. Place all of these projects in a directory. Compile all (the order above is recommended) by going into each one and running "make all".
