#!/usr/bin/python

import sys, getopt
import string
import os
import re
import shutil
import glob
from os import path
from struct import *

def check_opts(oem_mark, html_folder, output_folder):
    #  to check oem_mark between a-z,A-Z
    if not oem_mark in string.uppercase[:26]:
        print 'pls check oem_mark is between A to Z !'
        exit(-2)

    #  to check html_folder is exist
    if not os.path.exists(html_folder):
        print html_folder,'is not exist'
        exit(-3)

    #  to check output_folder is exist ? if exist, to remove it
    if os.path.exists(output_folder):
        shutil.rmtree(output_folder)
        #  os.mkdir(output_folder)

    #  copy html_folder to output_folder
    shutil.copytree(html_folder, output_folder)
        
def cfg_get_opts(cfg_file, cfg_dict, dict_item):
    txt = cfg_file.readline()
    while txt.__len__() != 0:
        if not txt.isspace() and not txt[0] ==';':
            cfg_dict[dict_item] = txt[:-2]
            return
        txt = cfg_file.readline()

def get_cfg(prefix, data_line):
    html_name = ''
    dhtml_name = ''
    txt = re.search(prefix+'(_*)[TSIC]-[a-zA-Z0-9_]+', data_line)
    if txt != None:
        html_name = txt.group(0)
        #  print 'html_name:',html_name
    txt = re.search('<!--# echo var=\"(.+)\"-->', data_line)
    if txt != None:
        dhtml_name = txt.group(0)
        #  print 'dhtml_name:',dhtml_name

    if html_name == None or dhtml_name == None:
        return None
    else:
        return {html_name:dhtml_name}

def read_config():
    if not os.path.exists(os.getcwd()+'/'+'HTTP.CFG'):
        print 'HTTP.CFG is not exist !'

    cfg_file = open('HTTP.CFG')

    cfg_dict = {}

    cfg_get_opts(cfg_file, cfg_dict, 'prefix')
    cfg_get_opts(cfg_file, cfg_dict, 'rm_opts')
    cfg_get_opts(cfg_file, cfg_dict, 'compress_space')

    txt = cfg_file.readline()
    while txt.__len__() != 0:
        if not txt.isspace() and not txt[0] ==';':
            cfg_dict.update(get_cfg(cfg_dict['prefix'], txt))
        txt = cfg_file.readline()

    print 'cfg_count:', cfg_dict.__len__()
    print 'prefix:', cfg_dict['prefix']
    print 'rm_opts:', cfg_dict['rm_opts']
    print 'compress_space:', cfg_dict['compress_space']
    cfg_file.close()
    return cfg_dict

def replace_cgi(cfg_dict, filename, output_folder):
    output_filename = output_folder + '/' + os.path.basename(filename)
    #  print 'output_filename:', output_filename
    html_file = open(filename)
    html_wfile = open(output_filename, 'w')

    txt = html_file.readline()
    while txt.__len__() != 0:
        txt = txt.lstrip()
        txt = txt.strip('\t')
        cgi_seeker = re.search(cfg_dict['prefix']+'(_*)[TSIC]-[a-zA-Z0-9_]+', txt)
        #  if cgi_seeker != None:
        #          item_str = cgi_seeker.group()
        #          new_txt = txt.replace(item_str, cfg_dict.get(item_str))
        #          html_wfile.write(new_txt)
        #  else:
        #      html_wfile.write(txt)
        while cgi_seeker != None:
            item_str = cgi_seeker.group()
            txt = txt.replace(item_str, cfg_dict.get(item_str))
            cgi_seeker = re.search(cfg_dict['prefix']+'(_*)[TSIC]-[a-zA-Z0-9_]+', txt)
        html_wfile.write(txt)

        txt = html_file.readline()

    html_file.close()
    html_wfile.close()

"""
typedef struct {
    unsigned int StartFileList; //H
    unsigned int NextDirList;   //H
    unsigned char DirName[13];  //s
} DIR_LIST;

typedef struct {
    unsigned int StartFileData;
    unsigned int FileSize;
    unsigned int NextFileList;
    unsigned char FileName[13];
} FILE_LIST;
"""
def generate_files_list(dir_name, file_content):
    files_list = []
    allow_types = ['.HTM', '.htm', '.js', '.css', '.jpg', '.gif', '.GIF']
    files_name_list = glob.glob(dir_name+'/*.*')
    files_name_list.sort()
    #  print files_name_list
    for file_name in files_name_list:
        base_name, extend = path.splitext(file_name)
        if extend in allow_types:
            file_fd = open(file_name, 'rb')
            file_str = file_fd.read()
            if extend == '.HTM' or extend == '.htm':
                file_str = file_str.replace('\r\n', '\n')
            file_content.append(file_str)
            file_fd.close()
            files_list.append({'start_file_data':0, 
                'file_size':file_content[-1].__len__(),
                'next_file_list': 13+path.basename(file_name).__len__(), 
                'file_name':path.basename(file_name)})
            #  print 'name:',file_name,'size:',file_content[-1].__len__()

    return files_list

def relink(dirs_list, files_list, file_content):

    #  dirs_list_size
    #  files_list_size[0]
    #  file_content_size[0]
    #  files_list_size[1]
    #  file_content_size[1]
    #  ....
    #  get dirs_list size

    dname_size = 0
    files_list_size = []
    for idx, dir_list in enumerate(dirs_list):
        dname_size += dir_list['dir_name'].__len__() + 1
    dirs_list_size = dname_size + 8 * (idx+1)

    #  get files_list size
    for idx, file_list in enumerate(files_list):
        fname_size = 0
        for fidx, file_info in enumerate(file_list):
            fname_size += file_info['file_name'].__len__() + 1
        files_list_size.append((fidx+1) * 12 + fname_size)

    file_list_addr  = dirs_list_size
    content_addr    = file_list_addr
    for idx, dir_info in enumerate(dirs_list):

        #  dir_list_unit = {'start_file_list':0, 'next_dir_list':0, 'dir_name':''}
        #  file_list_unit = {'start_file_data':0, 'file_size':0, 'next_file_list':0, 'file_name':''}
        dir_info['start_file_list'] = file_list_addr
        content_addr += files_list_size[idx]
        for fidx, file_info in enumerate(files_list[idx]):
            file_info['start_file_data'] = content_addr
            file_info['next_file_list'] = file_list_addr + 13 + file_info['file_name'].__len__()

            content_addr += file_info['file_size']
            file_list_addr = file_info['next_file_list']
        file_info['next_file_list'] = 0
        file_list_addr = content_addr

def www_to_bin(output_folder, file_buf):
    dir_name_list = []
    file_name_list = []
    dirs_list = []
    files_list = []
    data_pos = 0
    file_content = []

    #  dir_list_unit = {'start_file_list':0, 'next_dir_list':0, 'dir_name':''}
    #  file_list_unit = {'start_file_data':0, 'file_size':0, 'next_file_list':0, 'file_name':''}

    all_list = glob.glob(output_folder+'/*')

    dir_name_list.append(output_folder)
    for fname in all_list:
        if path.isfile(fname):
            file_name_list.append(fname)
        else:
            dir_name_list.append(fname)

    #  generate dirs_list struct
    for indx,dir_name in enumerate(dir_name_list):
        dirs_list.append({'start_file_list':0, 'next_dir_list':0, 'dir_name':path.basename(dir_name)})

        if path.basename(output_folder) == dirs_list[-1].get('dir_name'):
           dirs_list[-1]['dir_name'] = '.' 

        data_pos += (9 + dirs_list[-1].get('dir_name').__len__())
        dirs_list[-1]['next_dir_list'] = data_pos
        
        #  generate file_list by per dirs_list
        file_content.append([])
        files_list.append(generate_files_list(dir_name, file_content[-1]))

    dirs_list[-1]['next_dir_list'] = 0
    relink(dirs_list, files_list, file_content)

    #  collect all data
    for indx, dir_info in enumerate(dirs_list):
        file_buf.append(pack('<II', dir_info['start_file_list'], dir_info['next_dir_list']) + 
                dir_info['dir_name'] + '\0')
        
    for indx, files_info in enumerate(files_list):
        for findx, file_info in enumerate(files_info):
            file_buf.append(pack('<III', file_info['start_file_data'],
                file_info['file_size'],
                file_info['next_file_list']) +
                file_info['file_name'] + '\0')
        for findx, fcontent in enumerate(file_content[indx]):
            file_buf.append(fcontent)

    
def add_auth_file_list(file_path, file_buf):
    auth_txt = []
    auth_size = 0
    file_name = file_path + '/WEBAUTH.TXT'
    auth_file = open(file_name,'r')
    txt = auth_file.readline()

    while txt.__len__() != 0:
        if re.search('(/+)(.+)', txt) == None:
            auth_list = re.search('(\{-)?\w+[.-](\}|\w+)', txt)
            if auth_list != None:
                auth_txt.append(auth_list.group(0) + '\0')
                auth_size += auth_txt[-1].__len__()

        txt = auth_file.readline()
    auth_file.close()

    auth_txt.append('\0')
    auth_size += 3

    file_buf.append(pack('<h', auth_size))
    for txt in auth_txt:
        file_buf.append(txt)


def add_web_msg(file_path, file_buf):
    state = 0
    web_txt = []
    error_txt = []
    file_name = file_path + '/WEBMSG.TXT'
    msg_file = open(file_name, 'r')
    txt = msg_file.readline()

    while txt.__len__() != 0:
        if re.search('\[WebMsg\]', txt):
            state = 1
        if re.search('\[ErrMsg', txt):
            state = 2
        web_msg = re.search('\([0-9]+\) \"(.+)\"', txt)
        if web_msg != None:
            if state == 1:
                web_txt.append(web_msg.group(0)[6:-1]+'\0')
            if state == 2:
                error_txt.append(web_msg.group(0)[6:-1]+'\0')

        txt = msg_file.readline()

    msg_file.close()
    file_buf.append(pack('b', web_txt.__len__()))
    file_buf.append(pack('b', error_txt.__len__()))
    for msg in web_txt:
        file_buf.append(msg)
    for msg in error_txt:
        file_buf.append(msg)
    file_buf.append('\xff')

def bin_to_endmark():
    ints = []
    ss = []
    ebuf = []
    MAX_WWW_SIZE = 512 * 1024
    end_file = open('www.tmp','rb')
    bcontent = end_file.read()
    end_file.close()
    bsize = bcontent.__len__()

    if bsize < MAX_WWW_SIZE:
        zsize = MAX_WWW_SIZE - bsize
    else:
        print 'file size is too big ! (MAX_WWW_SIZE = %d)' % MAX_WWW_SISZE
        return

    bcontent = bcontent + '\0' * zsize
        
    ebuf.append('const char EndMark[]="{EndMark}";\n')
    ebuf.append('const unsigned long EndMark1 = 0x00544F5A;\n')
    ebuf.append('const unsigned long MyDataSize[4] = { %d, %d, %d, 0 };\n'%(
        MAX_WWW_SIZE,
        0,
        bsize
        ))
    ebuf.append('const unsigned char MyData[%d] = { \n' % MAX_WWW_SIZE)

    hex_m = '0x%02X,' * 16 + '\n'
    addr_m = '/* ' + '%05X '
    char_m = '%c' * 16 + ' */ '

    for pos in range(0, bcontent.__len__()):
        ints.append(ord(bcontent[pos]))
        ss.append(bcontent[pos])
        if ints[pos] <= 0x1f or ints[pos] >= 0x7f:
            ss[-1] = '.'
        if bcontent[pos] == '/' or bcontent[pos] == '\\':
            ss[-1] = '.'
            

    for pos in range(0,ints.__len__(), 16):
        ebuf.append(addr_m % pos)
        ebuf.append(char_m % tuple(ss[pos:pos+16]))
        ebuf.append(hex_m % tuple(ints[pos:pos+16]))
        #  print char_m % tuple(ss[pos:pos+16])
        #  print hex_m % tuple(ints[pos:pos+16])
    ebuf.append('};\n')
    
    end_file = open('end_mark.c', 'w')
    for pos in range(0, ebuf.__len__()):
        end_file.write(ebuf[pos])
    end_file.close()

web_data = []

def main(argv):

    if (argv.__len__() != 3):
        print 'usage: www.py oem_mark html_folder output_folder'
        exit(-1)

    oem_mark        = argv[0]
    html_folder     = os.getcwd() + '/' + argv[1]
    output_folder   = os.getcwd() + '/' + argv[2]
    check_opts(oem_mark, html_folder, output_folder)

    #  to parser HTTP.CFG
    cfg_database = {}
    cfg_database = read_config()

    #  to replace zot_cgi
    for html_file in glob.glob(html_folder+'/*.HTM'):
        replace_cgi(cfg_database, html_file, output_folder)
    for html_file in glob.glob(html_folder+'/*.htm'):
        replace_cgi(cfg_database, html_file, output_folder)

    #  add web version and oem marker
    web_data.append('\2' + oem_mark + '\0\0')

    #  add web auth file list
    add_auth_file_list(html_folder, web_data)

    #  add http message file
    add_web_msg(html_folder, web_data)
    
    #  www to bin
    www_to_bin(output_folder, web_data)

    www_file = open('www.tmp', 'wb')
    for indx, fbuf in enumerate(web_data):
        www_file.write(fbuf)
    www_file.close()

    # web data convert to endmark.c
    bin_to_endmark()

if __name__ == "__main__":
    main(sys.argv[1:]);
