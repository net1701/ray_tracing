------------------------------------------------
RAY TRACING homework
------------------------------------------------
  Sigismondo Boschi
  Feb 10th, 2014
------------------------------------------------

The program must be compiled with "make" in the current directory. 
It must me run with:

$ bin/ray_tracing test.dat

You can find description of the option in the input file "test.dat" itself.

-------------------------------------------------
Missing refinements
-------------------------------------------------
- handling of numerical approximation for mostly tangent rays
- handling of the divergence with a 2 step algorithm
- better lightening control
- storing more temporary staff in shared memory (see intersection, for example)
- light distance dependency ignored
- environment background from spherical image on texture, as:
environment background mapped on a texture from:
http://www.flickr.com/photos/discocandy/4115210303/in/photostream/

