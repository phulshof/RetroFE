import argparse
import errno
import glob
import os
import shutil
import sys

def copytree(src, dst):
  if os.path.isdir(src):
      if not os.path.exists(dst):
          mkdir_p(dst)
      for name in os.listdir(src):
          copytree(os.path.join(src, name),
                   os.path.join(dst, name))
  else:
      print("COPY: " + dst)
      shutil.copyfile(src, dst)
      
def mkdir_p(path):
  print("CREATE: " + path)
  try:
    os.makedirs(path)
    
  except OSError as exception:
    if exception.errno != errno.EEXIST:
      raise
#todo: this script needs to be broken up into multiple methods 
#      and should be ported to be more like a class

#####################################################################
# Parse arguments
#####################################################################
parser = argparse.ArgumentParser(description='Bundle up some RetroFE common files.')
parser.add_argument('--os', choices=['windows','linux','mac'], required=True, help='Operating System (windows or linux or mac)')
parser.add_argument('--build', default='full', help='Define what contents to package (full, core, engine, layout, none')
parser.add_argument('--clean', action='store_true', help='Clean the output directory')
parser.add_argument('--compiler', help='Compiler to use (vs, mingw, gcc')

args = parser.parse_args()

#####################################################################
# Determine base path os to build
#####################################################################
base_path = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
common_path = os.path.join(base_path, 'Package', 'Environment', 'Common')
os_path = None

if args.os == 'windows':
  os_path = os.path.join(base_path, 'Package', 'Environment', 'Windows')
  
elif args.os == 'linux':
  os_path = os.path.join(base_path, 'Package', 'Environment', 'Linux')

elif args.os == 'mac':
  os_path = os.path.join(base_path, 'Package', 'Environment', 'MacOS')

#####################################################################
# Copy layers, artwork, config files, etc for the given os
#####################################################################
output_path = os.path.join(base_path, 'Artifacts', args.os, 'RetroFE')

if os.path.exists(output_path) and hasattr(args, 'clean'):
  shutil.rmtree(output_path)


if args.build != 'none' and not os.path.exists(output_path):
  os.makedirs(output_path)

if args.build == 'full':
  collection_path = os.path.join(output_path, 'collections')
  copytree(common_path, output_path)
  copytree(os_path, output_path)
  
  mkdir_p(os.path.join(output_path, 'meta', 'mamelist'))
  
  dirs = [d for d in os.listdir(collection_path) if os.path.isdir(os.path.join(collection_path, d))]
  for collection in dirs:
    if not collection.startswith('_'):
     mkdir_p(os.path.join(output_path, 'collections', collection, 'roms'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'artwork_front'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'artwork_back'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'medium_back'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'medium_front'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'bezel'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'logo'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'screenshot'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'screentitle'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'medium_artwork', 'video'))
     mkdir_p(os.path.join(output_path, 'collections', collection, 'system_artwork'))
 
elif args.build == 'layout':
  layout_dest_path = os.path.join(output_path, 'layouts')
  layout_common_path = os.path.join(common_path, 'layouts')
  layout_os_path = os.path.join(os_path, 'layouts')
  
  if not os.path.exists(layout_dest_path):
    os.makedirs(layout_dest_path)

  if os.path.exists(layout_common_path):
    copytree(layout_common_path, layout_dest_path)
    
  if os.path.exists(layout_os_path):
    copytree(layout_os_path, layout_dest_path)

#####################################################################
# Copy retrofe executable
#####################################################################
if args.os == 'windows':
  if args.build == 'full' or args.build == 'core' or args.build == 'engine':
    # copy retrofe.exe to core folder
    if(hasattr(args, 'compiler') and args.compiler == 'mingw'):
      src_exe = os.path.join(base_path, 'RetroFE', 'Build', 'retrofe.exe')
    else:
      src_exe = os.path.join(base_path, 'RetroFE', 'Build', 'Release', 'retrofe.exe')
      
    core_path = os.path.join(output_path, 'core')
    
    # create the core folder
    if not os.path.exists(core_path):
      os.makedirs(core_path)
      
    # copy retrofe.exe
    shutil.copy(src_exe, core_path)
#    third_party_path = os.path.join(base_path, 'RetroFE', 'ThirdParty')
        
elif args.os == 'linux':
  if args.build == 'full' or args.build == 'core' or args.build == 'engine':
    src_exe = os.path.join(base_path, 'RetroFE', 'Build', 'retrofe')
    shutil.copy(src_exe, output_path)

elif args.os == 'mac':
  if args.build == 'full' or args.build == 'core' or args.build == 'engine':
    src_exe = os.path.join(base_path, 'RetroFE', 'Build', 'retrofe')
    shutil.copy(src_exe, output_path)
    app_path = os.path.join(output_path, 'RetroFE.app')
    if not os.path.exists(app_path):
      copytree(os_path, output_path)
    shutil.copy(src_exe, output_path + '/RetroFE.app/Contents/MacOS/')



