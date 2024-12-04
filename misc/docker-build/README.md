This is intended to document how to build a release version of SFZQ.  For a
release, one wants to create something that will run under as many different
distros as possible.  That means using older versions of GLIBC and GCC.  The
Surge XT synthesizer project has helpfully shared how they do that, using
Docker; this is a modification of their work.

How to do it:

```sh
./make-image
./enter
```

The `make-image` only needs to be done once.  The `enter` fires up a container
and drops you into a shell there.  Once inside, you should be able to do this:

```sh
cd persistent/sfzq
make
```

Assuming all goes well, you can then exit the container, and the shippable
plugin will be at "persistent/sfzq/sfzq.clap".

