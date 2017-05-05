#! /usr/bin/python
import binascii
import sys
from array import array
from time import localtime, strftime
from struct import *

#  typedef struct _header_ver1{
#          DWORD crc32_chksum;      //crc32 checksum
#          char  mark[3]; 	    //"XGZ"
#          BYTE  bintype; 	    //0xB0: All Flash, 0xB1: Code1, 0xB2: Code2, 0xB3: EEPROM, 0xB4: LOADER
#          DWORD file_len;          //Total Image file lentch
#          BYTE  next;
#          BYTE  reserved1[19];
#          BYTE  MajorVer;
#          BYTE  MinorVer;
#          BYTE  model; 	    //Target model
#          BYTE  ReleaseVer;
#          DWORD RDVersion;
#          WORD  BuildVer;
#          BYTE  data[60];
#          BYTE  reserved2[154];
#  }  HEADER_VER1, *pHEADER_VER1;

class Header:
    def __init__(self, 
                 mark = ['X','G','Z'],
                 bin_type = 0,
                 file_len = 0,
                 major_ver = 0,
                 minor_ver = 0,
                 model = 0,
                 release_ver = 0,
                 rd_ver = 0,
                 build_ver = 0,
                 cpu_marker = 'STR8132'):
        self.mark = mark
        self.bin_type = bin_type
        self.file_len = file_len
        self.bin_next = 0
        self.reserved1 = 0 
        self.major_ver = major_ver
        self.minor_ver = minor_ver
        self.mode = model
        self.release_ver = release_ver
        self.rd_ver = rd_ver
        self.build_ver = build_ver
        self.reserved2 = 0
        self.cpu_marker = cpu_marker

    def set_mark(self, m0, m1, m2):
        self.mark = [m0, m1, m2]
    def set_bin_type(self, b_type):
        self.bin_type = b_type
    def set_file_len(self, flen):
        self.file_len = flen
    def version(self, major_ver, minor_ver):
        self.major_ver = int(major_ver)
        self.minor_ver = int(minor_ver)
    def model(self, model):
        self.model = model
    def release_version(self, r_ver):
        self.release_ver = r_ver
    def rd_version(self, rd_ver):
        self.rd_ver = int(rd_ver) - 1
    def build_version(self, b_ver):
        self.build_ver = b_ver
    def set_cpu_marker(self, cpu_marker):
        self.cpu_marker = cpu_marker
    def generate_header(self):

        time_str = strftime("%Y/%m/%d %H:%M:%S", localtime())
        frm_string = '%s-%d.%02d.%02d.%04d.%08d%c-%s'
        frm_string =  frm_string % (self.cpu_marker, 
                                    self.major_ver, 
                                    self.minor_ver, 
                                    self.model, 
                                    self.build_ver, 
                                    self.rd_ver, 
                                    self.release_ver, 
                                    time_str)
        self.data = frm_string
        header_str = '' 
        header_str += '%c%c%c' % (ord(self.mark[0]), ord(self.mark[1]), ord(self.mark[2]))
        header_str += pack('B', self.bin_type)
        header_str += pack('<I', self.file_len)
        header_str += '%c' % self.bin_next
        header_str += '%s' % ('\0'*19)
        header_str += pack('<BBBB', self.major_ver, self.minor_ver, self.model, self.release_ver)
        header_str += pack('<I', self.rd_ver)
        header_str += pack('<H', self.build_ver)
        firmware_str = '%s' % self.data
        oc_num = 60 - firmware_str.__len__()
        firmware_str += '\0' * oc_num
        header_str += firmware_str
        header_str += '%s' % ('\0'*154)
        return header_str


crc32table = [  0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
                0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
                0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
                0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
                0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
                0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
                0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
                0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
                0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
                0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
                0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
                0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
                0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
                0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
                0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
                0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
                0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
                0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
                0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
                0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
                0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
                0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
                0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
                0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
                0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
                0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
                0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
                0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
                0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
                0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
                0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
                0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
                0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
                0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
                0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
                0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
                0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
                0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
                0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
                0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
                0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
                0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
                0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
                0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
                0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
                0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
                0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
                0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
                0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
                0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
                0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
                0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
                0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
                0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
                0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
                0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
                0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
                0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
                0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
                0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
                0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
                0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
                0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
                0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D ]

def crc32(string):
    value = 0xffffffff
    for ch in string:
        value = crc32table[(ord(ch) ^ value) & 0x000000FF] ^ ((value >> 8) & 0x00FFFFFF)
    return value

def compute_crc(file_name):
    bin_file = open(file_name, 'rb')
    bin_buf = bin_file.read() 
    crc_buf = bin_buf[4:]
    #  crc_buf = crc_buf

    bin_file.close()
    return crc32(crc_buf)


def main(argv):

    if (argv.__len__() != 11):
        print 'usage: mkimage.py PS_MODEL BIN_TYPE MAJOR_VER MINOR_VER RELEASE_VER RD_VER BUILD_VER CPU_MARK X_MARK BIN_FILE OUT_FILE'
        print 'len:', argv.__len__()
        exit(-1)

    input_file_name = argv[9]
    output_file_name = argv[10]

    in_file = open(input_file_name, 'rb')
    file_content = in_file.read()
    in_file.close()

    image_header = Header()
    image_header.model(int(argv[0]))
    image_header.set_bin_type(int(argv[1],16))
    image_header.version(int(argv[2]), int(argv[3]))
    image_header.release_version(int(argv[4]))
    image_header.rd_version(int(argv[5]))
    image_header.build_version(int(argv[6]))
    image_header.set_cpu_marker(argv[7])
    image_header.set_mark('X',argv[8],'Z')
    image_header.set_file_len(file_content.__len__())

    bin_txt = image_header.generate_header()

    bin_txt += file_content
    crc_str = pack('<I', crc32(bin_txt))
    print 'crc = 0x%08x' % crc32(bin_txt)

    print 'input_file:', input_file_name
    print 'output_file:', output_file_name

    out_file = open(output_file_name, 'wb')
    out_file.write(crc_str + bin_txt)
    out_file.close()


if __name__ == "__main__":
    main(sys.argv[1:])
