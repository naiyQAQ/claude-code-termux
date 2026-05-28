/* glibc-to-bionic shim: libdl.so.2 - empty SONAME stub.
 * dlsym/dladdr version requirements are patched out of .node files
 * so bionic's unversioned dlsym/dladdr can be used directly. */
int __libdl_shim_placeholder = 0;
