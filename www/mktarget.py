#! /usr/bin/python
import sys
import re
from time import localtime, strftime

def get_value(key, txt):

    hit_txt = re.search(key+'\s+=(.+)', txt)
    if hit_txt != None:
        value_txt = hit_txt.group(0).split('=')
        return value_txt[1].strip()
    else:
        return None

def replace_value(key, new_value, txt):

    old_value = get_value(key, txt)

    hit_txt = re.search(key+'\s+=(.+)', txt)
    if hit_txt != None:
        old_txt = hit_txt.group(0)
        new_txt = old_txt.replace(old_value, new_value)
        return txt.replace(old_txt, new_txt)
    else:
        return None


# usage: mktarget.py model oem_html 
def main(argv):

    model = argv[0]
    oem_html = argv[1]

    target_file = open('../prod/'+ model +'/build/Target.def', 'r')
    def_txt = target_file.read()
    target_file.close()

    cpu_mark =  get_value('MAKER_AND_CPU', def_txt)
    model_index = get_value('PSMODELINDEX', def_txt)
    major_ver = get_value('MajorVer', def_txt)
    minor_ver = get_value('MinorVer', def_txt)
    release_ver = get_value('ReleaseVer', def_txt)
    rd_version = get_value('RDVersion', def_txt)
    build_ver = get_value('BuildVer', def_txt)


    # update oem_file
    new_txt = replace_value('HTML_FILE', oem_html, def_txt)

    # increase rd_version
    new_txt = replace_value('RDVersion', str(int(rd_version)+1), new_txt)

    time_str = strftime("%Y/%m/%d %H:%M:%S", localtime())
    frm_string = '(%s-%d.%02d.%02d.%04d.%08d%c-%s)'
    frm_string =  frm_string % (cpu_mark, int(major_ver), int(minor_ver), int(model_index), int(build_ver), int(rd_version)+1, int(release_ver), time_str)

    # udpate firmware string
    new_txt = replace_value('FirmwareString', frm_string, new_txt)

    target_file = open('../prod/'+ model +'/build/Target.def', 'w')
    target_file.write(new_txt)
    target_file.close()
    print new_txt

if __name__ == "__main__":
    main(sys.argv[1:]);
