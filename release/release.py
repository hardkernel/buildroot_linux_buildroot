#!/usr/bin/python

import sys
import re
import os
import time

base_url = 'http://openlinux.amlogic.com:8000/download/ARM/'

pkg = {
   'customer':'BR2_PACKAGE_AML_CUSTOMER_GIT_URL', 
   'kernel':'BR2_LINUX_KERNEL_CUSTOM_REPO_URL', 
   'gpu':'BR2_PACKAGE_GPU_GIT_URL',
   'wifi':'BR2_PACKAGE_WIFI_FW_CUSTOM_GIT_REPO_URL',
   '8188eu':'BR2_PACKAGE_RTK8188EU_GIT_REPO_URL',
   '8192cu':'BR2_PACKAGE_RTK8192CU_GIT_REPO_URL',
   '8192du':'BR2_PACKAGE_RTK8192DU_GIT_REPO_URL',
   '8192eu':'BR2_PACKAGE_RTK8192EU_GIT_REPO_URL',
   'ap6xxx':'BR2_PACKAGE_BRCMAP6XXX_GIT_REPO_URL',
   'tvin':'BR2_PACKAGE_AML_TVIN_GIT_REPO_URL',
   'pmu':'BR2_PACKAGE_AML_PMU_GIT_REPO_URL',
   'nand':'BR2_PACKAGE_AML_NAND_GIT_URL',
   'uboot':'BR2_TARGET_UBOOT_CUSTOM_REPO_URL', 
   'uboot_customer':'BR2_PACKAGE_AML_UBOOT_CUSTOMER_GIT_REPO_URL'
}

repos = { 
   'customer':'kernel/customer', 
   'kernel':'kernel/common', 
   'gpu':'platform/hardware/arm/gpu', 
   'wifi':'platform/hardware/amlogic/wifi', 
   '8188eu':'platform/hardware/wifi/realtek/drivers/8188eu', 
   '8192cu':'platform/hardware/wifi/realtek/drivers/8192cu', 
   '8192du':'platform/hardware/wifi/realtek/drivers/8192du', 
   '8192eu':'platform/hardware/wifi/realtek/drivers/8192eu', 
   'ap6xxx':'platform/hardware/wifi/broadcom/drivers/ap6xxx', 
   'tvin':'linux/amlogic/tvin', 
   'pmu':'platform/hardware/amlogic/pmu', 
   'nand':'platform/hardware/amlogic/nand', 
   'uboot':'uboot', 
   'uboot_customer':'uboot/customer'
}

branches = { 
    'customer':'BR2_PACKAGE_AML_CUSTOMER_VERSION', 
    'kernel':'BR2_LINUX_KERNEL_CUSTOM_REPO_VERSION', 
    'wifi':'BR2_PACKAGE_WIFI_CUSTOM_GIT_VERSION', 
    '8188eu':'BR2_PACKAGE_RTK8188EU_GIT_VERSION', 
    '8192cu':'BR2_PACKAGE_RTK8192CU_GIT_VERSION', 
    '8192du':'BR2_PACKAGE_RTK8192DU_GIT_VERSION', 
    '8192eu':'BR2_PACKAGE_RTK8192EU_GIT_VERSION', 
    'ap6xxx':'BR2_PACKAGE_BRCMAP6XXX_GIT_VERSION', 
    'tvin':'BR2_PACKAGE_AML_TVIN_GIT_VERSION',
    'pmu':'BR2_PACKAGE_AML_PMU_GIT_VERSION',
    'nand':'BR2_PACKAGE_AML_NAND_VERSION',
    'uboot':'BR2_TARGET_UBOOT_CUSTOM_REPO_VERSION',
    'uboot_customer':'BR2_PACKAGE_AML_UBOOT_CUSTOMER_GIT_VERSION'
}

tar = { 
   'customer':'aml_customer', 
   'kernel':'arm-src-kernel', 
   'gpu':'gpu', 
   'wifi':'wifi-fw', 
   '8188eu':'rtk8188eu', 
   '8192cu':'rtk8192cu', 
   '8192du':'rtk8192du', 
   '8192eu':'rtk8192eu', 
   'ap6xxx':'brcmap6xxx', 
   'tvin':'aml_tvin', 
   'pmu':'aml_pmu', 
   'nand':'aml_nand', 
   'uboot':'uboot', 
   'uboot_customer':'aml_uboot_customer'
}

location = { 
   'customer':'customer', 
   'kernel':'kernel', 
   'gpu':'gpu', 
   'wifi':'wifi', 
   '8188eu':'wifi', 
   '8192cu':'wifi', 
   '8192du':'wifi', 
   '8192eu':'wifi', 
   'ap6xxx':'wifi', 
   'tvin':'modules', 
   'pmu':'modules', 
   'nand':'modules', 
   'uboot':'u-boot', 
   'uboot_customer':'u-boot'
}

tarball_cfg = {
   'kernel':'BR2_LINUX_KERNEL_CUSTOM_TARBALL_LOCATION BR2_LINUX_KERNEL_CUSTOM_TARBALL', 
   'gpu':'BR2_PACKAGE_GPU_CUSTOM_TARBALL_LOCATION', 
   'uboot':'BR2_TARGET_UBOOT_CUSTOM_TARBALL_LOCATION BR2_TARGET_UBOOT_CUSTOM_TARBALL'
}

drop = {
        'kernel':'BR2_LINUX_KERNEL_CUSTOM_GIT',
        'uboot':'BR2_TARGET_UBOOT_CUSTOM_GIT'
}

filename = dict()

def getserver(string):
    server = re.compile('<remote fetch=\"(.*)\" name=.*/>') 
    string = string.strip()
    loc = re.match(server, string)
    if loc != None:
        return loc.group(1)

def getpkg(string):
    rev = re.compile('<project name=\"([a-z0-9/]*)\".* revision=\"([a-z0-9]*)\" upstream=\"(.*)\"/>')
    string = string.strip()
    loc = re.match(rev, string)
    if loc != None:
        return loc.group(1), loc.group(2), loc.group(3)

def download_pkg(xml, config, download = 0):
    server_addr = None
    for pkgline in open(config, 'r'):
        for i in pkg.keys():
            if pkg[i] in pkgline:
              for line in open(xml, 'r'):
                  if server_addr == None:
                      server_addr = getserver(line)
                  else:
                      if repos[i] in line:
                          name, pkgloc, b = getpkg(line)
                          if name != repos[i]:
                              break
                          server = server_addr + repos[i]
                          date = time.strftime("%Y-%m-%d")
                          folder = tar[i] + '-' + date + '-' + pkgloc[0:10]
                          if download:
                              cmd = 'rm -rf %s' % folder
                              print cmd
                              os.system(cmd)
                              cmd = 'git clone --depth=0 %s -b %s %s' % (server, b, folder)
                              print cmd
                              os.system(cmd)
                              cmd = 'cd %s; git archive --format=tar.gz %s --prefix=%s/ -o %s.tar.gz' % (folder, pkgloc, folder, folder)
                              print cmd
                              os.system(cmd)
                              cmd = 'mv %s/%s.tar.gz .' % (folder, folder)
                              print cmd
                              os.system(cmd)
                          filename[i] = folder + ".tar.gz"

def create_cfg(config):
    newfile = config.replace('defconfig', 'release_defconfig', 1)
    f = open(newfile, 'w')
    for pkgline in open(config, 'r'):
        for i in pkg.keys():
            if pkg[i] in pkgline:
                if tarball_cfg.has_key(i) != False:
                    cfg = tarball_cfg[i].split()
                    new_cfg = '%s=\"%s\"\n' % (cfg[0], base_url + location[i] + "/" + filename[i])
                    check = None
                    if len(cfg) > 1:
                        check = '%s=y\n' % cfg[1]
                        f.write(check)
                    f.write(new_cfg)
                else:
                    cfg = pkgline.split('=')
                    new_cfg = '%s=\"%s\"\n' % (cfg[0], base_url + location[i])
                    version = filename[i].replace(tar[i] + '-', '', 1)
                    version = version.replace('.tar.gz', '', 1)
                    version = '%s=\"%s\"\n' % (branches[i], version)
                    f.write(version)
                    f.write(new_cfg)
                break
            elif i in branches and branches[i] in pkgline:
                break
            elif i in drop and drop[i] in pkgline:
                break
        else:
            f.write(pkgline)
    f.close()

if __name__ == '__main__':
    xml = sys.argv[1]
    cfg = sys.argv[2]
    download = len(sys.argv) == 4 and sys.argv[3] == "download"
    download_pkg(xml, cfg, download)
    create_cfg(cfg)

