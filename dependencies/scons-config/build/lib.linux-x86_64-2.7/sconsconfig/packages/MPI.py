
import sys, os
from Package import Package
import subprocess

class MPI(Package):

  def __init__(self, **kwargs):
    defaults = {
      'download_url': 'http://www.mcs.anl.gov/research/projects/mpich2/downloads/tarballs/1.4.1p1/mpich2-1.4.1p1.tar.gz',
    }
    defaults.update(kwargs)
    super(MPI, self).__init__(**defaults)
    
    self.check_text = r'''
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
int main(int argc, char* argv[])
{
  int rank;
  MPI_Init(&argc, &argv);
  printf("%d\n", MPI_VERSION);
  printf("%d\n", MPI_SUBVERSION);

  /* This will catch OpenMPI libraries. */
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Finalize();
  return EXIT_SUCCESS;
}
'''


  def check(self, ctx):
    env = ctx.env
    ctx.Message('Checking for MPI ... ')
    self.check_options(env)

    # on hazel hen login node do not run MPI test program because this is not possible (only compile)
    if os.environ.get("SITE_PLATFORM_NAME") == "hazelhen":
      self.run = False
      
    try:
      # try to get compiler and linker flags from mpicc, this directly has the needed includes paths
      
      cflags = subprocess.check_output("mpic++ --showme:compile", shell=True)
      ldflags = subprocess.check_output("mpic++ --showme:link", shell=True)

      # remove trailing newline
      if cflags[-1] == '\n':
        cflags = cflags[:-1]
      if ldflags[-1] == '\n':
        ldflags = ldflags[:-1]

      ctx.Log("extracted cflags from mpic++: \n{}\n\n".format(cflags))
      ctx.Log("extracted ldflags from mpic++: \n{}\n\n".format(ldflags))

      env.MergeFlags(cflags)
      env.MergeFlags(ldflags)
      
      res = self.try_link(ctx)
      
    except Exception as e: 
      ctx.Message("MPI mpic++ --showme failed: \n"+str(e)+"\nNow using MPI_DIR\n")
      
      # mpicc was not available (e.g. on hazel hen), now try to use the MPI_DIR variable, as usual
      self.headers = ['mpi.h']
      self.libs=[
        ['mpich'],                   # mpich/mpich2
        ['pmpich', 'mpich'],         # mpich2
        ['mpich', 'mpl'],            # mpich2
        ['pmpich', 'mpich', 'mpl'],  # mpich2
        ['mpi', 'mpi_cxx'],          # openmpi
        ['mpi'],                     # openmpi
      ]
      self.extra_libs=[
        [],
        ['rt'],
        ['pthread', 'rt'],
        ['dl'],
        ['dl', 'rt'],
        ['dl', 'pthread'],
        ['dl', 'pthread', 'rt']
      ]
      res = super(MPI, self).check(ctx)
    
    self.check_required(res[0], ctx)
    ctx.Result(res[0])
    return res[0]
