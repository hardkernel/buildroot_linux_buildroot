#!/usr/bin/env python
#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# aurthor: zhigang.yu@amlogic.com

import sys
reload(sys)
sys.setdefaultencoding('utf-8')

if sys.hexversion < 0x02040000:
  print >> sys.stderr, "Python 2.4 or newer is required."
  sys.exit(1)

import copy
import errno
import os
import re
import subprocess
import tempfile
import time
import zipfile
import shutil

try:
  from hashlib import sha1 as sha1
except ImportError:
  from sha import sha as sha1

tempfiles = []

def Run(args, **kwargs):
  """Create and return a subprocess.Popen object, printing the command
  line on the terminal if -v was specified."""

  print "  running: ", " ".join(args)
  return subprocess.Popen(args, **kwargs)

def ZipWriteStr(zip, filename, data, perms=0644):
  # use a fixed timestamp so the output is repeatable.
  zinfo = zipfile.ZipInfo(filename=filename,
                          date_time=(2009, 1, 1, 0, 0, 0))
  zinfo.compress_type = zip.compression
  zinfo.external_attr = perms << 16
  zip.writestr(zinfo, data)

def WriteLocalFile(name, data):
  fp = open(name,"wb+")
  fp.write(data)
  fp.flush()
  fp.close()

class File(object):
  def __init__(self, name, data):
    self.name = name
    self.data = data
    self.size = len(data)
    self.sha1 = sha1(data).hexdigest()

  @classmethod
  def FromLocalFile(cls, name, diskname):
    f = open(diskname, "rb")
    data = f.read()
    f.close()
    return File(name, data)

  def WriteToTemp(self):
    t = tempfile.NamedTemporaryFile()
    t.write(self.data)
    t.flush()
    return t

  def AddToZip(self, z):
    ZipWriteStr(z, self.name[7:], self.data)

def UnzipTemp(filename, pattern=None):
  """Unzip the given archive into a temporary directory and return the name.

  If filename is of the form "foo.zip+bar.zip", unzip foo.zip into a
  temp dir, then unzip bar.zip into that_dir/BOOTABLE_IMAGES.

  Returns (tempdir, zipobj) where zipobj is a zipfile.ZipFile (of the
  main file), open for reading.
  """

  tmp = tempfile.mkdtemp(prefix="targetfiles-")
  tempfiles.append(tmp)

  def unzip_to_dir(filename, dirname):
    cmd = ["unzip", "-o", "-q", filename, "-d", dirname]
    if pattern is not None:
      cmd.append(pattern)
    p = Run(cmd, stdout=subprocess.PIPE)
    p.communicate()
    if p.returncode != 0:
      raise ExternalError("failed to unzip input target-files \"%s\"" %
                          (filename,))

  m = re.match(r"^(.*[.]zip)\+(.*[.]zip)$", filename, re.IGNORECASE)
  if m:
    unzip_to_dir(m.group(1), tmp)
    unzip_to_dir(m.group(2), os.path.join(tmp, "BOOTABLE_IMAGES"))
    filename = m.group(1)
  else:
    unzip_to_dir(filename, tmp)

  return tmp, zipfile.ZipFile(filename, "r")

def Cleanup():
  for i in tempfiles:
    if os.path.isdir(i):
      shutil.rmtree(i)
    else:
      os.remove(i)

def IsSymlink(info):
  """Return true if the zipfile.ZipInfo object passed in represents a
  symlink."""
  return (info.external_attr >> 16) == 0120777

def LoadSystemFiles(z):
  """Load all the files from rootfs/... in a given target-files
  ZipFile, and return a dict of {filename: File object}."""
  out = {}
  for info in z.infolist():
    if info.filename.startswith("rootfs/") and not IsSymlink(info):
      basefilename = info.filename[7:]
      fn = "rootfs/" + basefilename
      data = z.read(info.filename)
      out[fn] = File(fn, data)
  return out

def ClosestFileMatch(src, tgtfiles):
  """Returns the closest file match between a source file and list
     of potential matches.  The exact filename match is preferred,
     then the sha1 is searched for, and finally a file with the same
     basename is evaluated.  Rename support in the updater-binary is
     required for the latter checks to be used."""

  result = tgtfiles.get("path:" + src.name)
  if result is not None:
    return result

def update_shell_start(output_dir, name, sha_s, sha_d):
  f=open(output_dir+"/update.sh",'a')
  tmp="sha1sum /tmp" + name
  command="	value_tmp" + "=$(" + tmp + ")"
  print command
  command0="	if [ " + "${value_tmp:0:40} != \"" + sha_s + "\" ];then"
  command1="		if [ " + "${value_tmp:0:40} != \"" + sha_d + "\" ];then"
  print command0
  command2="			exit 1"
  print command1
  command3="		fi"
  command4="	fi"
  f.write(command+"\n")
  f.write(command0+"\n")
  f.write(command1+"\n")
  f.write(command2+";\n")
  f.write(command3+"\n")
  f.write(command4+"\n")
  f.flush()
  f.close()

def update_shell_preinst(output_dir):
  f=open(output_dir+"/update.sh",'a')
  f.write("	echo increment check success!\n")
  f.write("fi"+"\n")
  f.write("\n")
  f.write("if [ $1 == "+ '"postinst"' " ]; then"+"\n")
  f.write("	echo -----------swupdate update.sh postinst---------------------\n")
  f.write("	unzip -o /tmp/rootfs.zip -d /tmp/rootfs;"+"\n")
  f.flush()
  f.close()

def update_shell_postinst(output_dir, cmd):
  f=open(output_dir+"/update.sh",'a')
  f.write("	" + cmd + "\n")
  f.flush()
  f.close()

def update_shell_end(output_dir):
  f=open(output_dir+"/update.sh",'a')
  f.write("	umount /tmp/rootfs/etc"+"\n")
  f.write("	umount /tmp/rootfs/var"+"\n")
  f.write("	umount /tmp/rootfs"+"\n")
  f.write("	rm /tmp/rootfs -rf"+"\n")
  f.write("	rm /tmp/rootfs.zip -rf"+"\n")
  f.write("fi"+"\n")
  f.flush()
  f.close()

def CopySystemFiles(input_zip, output_zip=None,
                    substitute=None):
  """Copies files underneath system/ in the input zip to the output
  zip.  Populates the Item class with their metadata, and returns a
  list of symlinks.  output_zip may be None, in which case the copy is
  skipped (but the other side effects still happen).  substitute is an
  optional dict of {output filename: contents} to be output instead of
  certain input files.
  """

  symlinks = []

  for info in input_zip.infolist():
    if info.filename.startswith("rootfs/"):
      basefilename = info.filename[7:]
      if IsSymlink(info):
        symlinks.append((input_zip.read(info.filename),
                         "/rootfs/" + basefilename))
  symlinks.sort()
  return symlinks

def build_sw_image(target_zip, output_dir):
  cmd = "chmod +x " + output_dir + "/increment.sh"
  os.system(cmd)
  if "u-boot.bin" in target_zip.namelist():
    cmd = "./" + output_dir + "/increment.sh" + " " + output_dir + " emmc"
  else:
    cmd = "./" + output_dir + "/increment.sh" + " " + output_dir + " nand"
  os.system(cmd)

def WriteIncrementalOTAPackage(target_zip, source_zip, output_zip, output_dir):

  print "Loading target..."
  target_data = LoadSystemFiles(target_zip)
  print "Loading source..."
  source_data = LoadSystemFiles(source_zip)

  WriteLocalFile(output_dir+"/boot.img", target_zip.read("boot.img"))
  WriteLocalFile(output_dir+"/dtb.img", target_zip.read("dtb.img"))
  WriteLocalFile(output_dir+"/update.sh", target_zip.read("update.sh"))
  WriteLocalFile(output_dir+"/sw-description", target_zip.read("sw-description"))
  WriteLocalFile(output_dir+"/increment.sh", target_zip.read("increment.sh"))
  WriteLocalFile(output_dir+"/swupdate-priv.pem", target_zip.read("swupdate-priv.pem"))

  if "u-boot.bin" in target_zip.namelist():
    WriteLocalFile(output_dir+"/u-boot.bin", target_zip.read("u-boot.bin"))
  else:
    WriteLocalFile(output_dir+"/u-boot.bin.usb.bl2", target_zip.read("u-boot.bin.usb.bl2"))
    WriteLocalFile(output_dir+"/u-boot.bin.usb.tpl", target_zip.read("u-boot.bin.usb.tpl"))

  matching_file_cache = {}
  patch_list = []
  file_delete = []

  for fn in source_data.keys():
    sf = source_data[fn]
    assert fn == sf.name
    matching_file_cache["path:" + fn] = sf
    # Only allow eligability for filename/sha matching
    # if there isn't a perfect path match.
    if target_data.get(sf.name) is None:
      ## rm
      print "++++++++rm %s\n " % (sf.name)
      file_delete.append(sf.name)

  for fn in sorted(target_data.keys()):
    tf = target_data[fn]
    assert fn == tf.name
    sf = ClosestFileMatch(tf, matching_file_cache)

    if sf is None:
      # add
      print "++++++++add %s\n " % (tf.name)
      tf.AddToZip(output_zip)
    elif tf.sha1 != sf.sha1:
      # diff
      tf.AddToZip(output_zip)
      patch_list.append((sf.name, tf, sf))
      print "++++++++diff %s\n " % (tf.name)
    else:
      # Target file data identical to source (may still be renamed)
      pass

  for fn, tf, sf in patch_list:
    update_shell_start(output_dir, "/"+fn, sf.sha1, tf.sha1)

  update_shell_preinst(output_dir)

  for df in file_delete:
    update_shell_postinst(output_dir, "rm " + df + " -rf")

  target_symlinks = CopySystemFiles(target_zip, None)
  target_symlinks_d = dict([(i[1], i[0]) for i in target_symlinks])

  source_symlinks = CopySystemFiles(source_zip, None)
  source_symlinks_d = dict([(i[1], i[0]) for i in source_symlinks])

  # Delete all the symlinks in source that aren't in target.  This
  # needs to happen before verbatim files are unpacked, in case a
  # symlink in the source is replaced by a real file in the target.
  to_delete = []
  for dest, link in source_symlinks:
    if link not in target_symlinks_d:
      to_delete.append(link)
      print "++++++++++++++++delete %s" % (link)
      update_shell_postinst(output_dir, "rm " + link)
  #script.DeleteFiles(to_delete)

  # Create all the symlinks that don't already exist, or point to
  # somewhere different than what we want.  Delete each symlink before
  # creating it, since the 'symlink' command won't overwrite.
  to_create = []
  for dest, link in target_symlinks:
    if link in source_symlinks_d:
      if dest != source_symlinks_d[link]:
        to_create.append((dest, link))
        print "++++++++++++++++link dest:%s  link:%s" % (dest, link)
        update_shell_postinst(output_dir, "rm " + link)
        update_shell_postinst(output_dir, "ln -s  " + dest + " " + link)
    else:
      to_create.append((dest, link))
      print "++++++++++++++++link dest:%s  link:%s" % (dest, link)
      update_shell_postinst(output_dir, "ln -s  " + dest + " " + link)

  update_shell_end(output_dir)
  build_sw_image(target_zip, output_dir)

def main(argv):
  try:
    print argv[0];
    print argv[1];
    print argv[2];
  except IndexError:
    print('\nUsage: ./ota_from_target_files.py <old_zip> <new.zip> <out_dir>\n')
    print('\nUsage: ./ota_from_target_files.py target_ota_180110.zip target_ota_180124.zip out_dir\n')

  if os.path.isdir(argv[2]):
    print "unzipping source target-files..."
    source_tmp, source_zip = UnzipTemp(argv[0])
    print "unzipping target target-files..."
    target_tmp, target_zip = UnzipTemp(argv[1])

    #temp_zip_file = tempfile.NamedTemporaryFile()
    #print 'temp_zip_file:', temp_zip_file
    #print 'temp_zip_file.name:', temp_zip_file.name
    output_zip = zipfile.ZipFile(argv[2] + "/rootfs.zip", "w",
                               compression=zipfile.ZIP_DEFLATED)
    WriteIncrementalOTAPackage(target_zip, source_zip, output_zip, argv[2])

    output_zip.close()
    #shutil.copy(temp_zip_file.name, argv[2] + "/rootfs.zip")
    #temp_zip_file.close()
  else:
    print "%s is not a dir" % (argv[2])

  Cleanup()

  print "done."

if __name__ == '__main__':
  try:
    main(sys.argv[1:])
  except KeyError:
    print
    sys.exit(1)