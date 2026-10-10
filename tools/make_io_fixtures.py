import struct,zlib,lzma,ctypes,ctypes.util,binascii,zipfile,argparse
from pathlib import Path
parser=argparse.ArgumentParser();parser.add_argument('output',type=Path);parser.add_argument('--zstd-library',default=ctypes.util.find_library('zstd'));args=parser.parse_args();out=args.output;out.mkdir(parents=True,exist_ok=True)
if not args.zstd_library: parser.error('provide --zstd-library for the reference Zstd encoder')
zstd=ctypes.CDLL(args.zstd_library);zstd.ZSTD_compressBound.argtypes=[ctypes.c_size_t];zstd.ZSTD_compressBound.restype=ctypes.c_size_t;zstd.ZSTD_compress.argtypes=[ctypes.c_void_p,ctypes.c_size_t,ctypes.c_void_p,ctypes.c_size_t,ctypes.c_int];zstd.ZSTD_compress.restype=ctypes.c_size_t
be=lambda n,k:n.to_bytes(k,'big')
def rawzip(b):
 c=zlib.compressobj(6,wbits=-15);return c.compress(b)+c.flush()
def zst(b):
 dst=ctypes.create_string_buffer(zstd.ZSTD_compressBound(len(b)));n=zstd.ZSTD_compress(dst,len(dst),b,len(b),3);assert n<len(dst);return dst.raw[:n]
def lzm(b):return lzma.compress(b,format=lzma.FORMAT_RAW,filters=[{'id':lzma.FILTER_LZMA1,'dict_size':32768,'lc':3,'lp':0,'pb':2}])
def crc(data,poly,width):
 v=0
 for x in data:
  v^=x<<(width-8)
  for _ in range(8):v=((v<<1)^poly if v&(1<<(width-1)) else v<<1)&((1<<width)-1)
 return v
def flac(b,framesize=2352):
 # Independent verbatim stereo frames, fixed block-size strategy.
 out=b''
 for n,start in enumerate(range(0,len(b),framesize*4)):
  block=b[start:start+framesize*4];samples=len(block)//4
  head=bytes([255,248,0x79,0x18,n])+be(samples-1,2)
  head+=bytes([crc(head,7,8)])
  frame=head
  for channel in [0,1]:frame+=bytes([2])+b''.join(block[i+channel*2:i+channel*2+2] for i in range(0,len(block),4))
  out+=frame+be(crc(frame,0x8005,16),2)
 return out
hb=8*2448
raw=b''.join(bytes((k*17+i*13)&255 for i in range(2352))+bytes(96) for k in range(32))
meta=b'TRACK:1 TYPE:AUDIO SUBTYPE:NONE FRAMES:32 PREGAP:0 PGTYPE:V PGSUB:NONE POSTGAP:0\0'
chunks=[raw[i:i+hb] for i in range(0,len(raw),hb)]
for name in ['none','zlib','lzma','zstd','cdzl','cdlz','cdzs','flac','cdfl']:
 blobs=[]
 for chunk in chunks:
  if name=='none':blob=chunk
  elif name=='zlib':blob=rawzip(chunk)
  elif name=='lzma':blob=lzm(chunk)
  elif name=='zstd':blob=zst(chunk)
  elif name=='flac':blob=b'B'+flac(chunk)
  else:
   sec=b''.join(chunk[i:i+2352] for i in range(0,hb,2448));sub=b''.join(chunk[i+2352:i+2448] for i in range(0,hb,2448))
   if name=='cdfl':blob=flac(sec)+rawzip(sub)
   else:
    comp={'cdzl':rawzip,'cdlz':lzm,'cdzs':zst}[name];base=comp(sec);blob=bytes(1)+be(len(base),2)+base+(zst(sub) if name=='cdzs' else rawzip(sub))
  blobs.append(blob)
 # Flat four-bit canonical tree, four entries, then length and CRC fields.
 bits='0100'*16+('0100' if name=='none' else '0000')*4
 for chunk,blob in zip(chunks,blobs):
  if name!='none':bits+=format(len(blob),'024b')
  bits+=format(binascii.crc_hqx(chunk,0xffff),'016b')
 bits+='0'*((-len(bits))%8);mapbody=int(bits,2).to_bytes(len(bits)//8,'big')
 mo=124;metaoff=mo+16+len(mapbody);datastart=metaoff+16+len(meta)
 packed=b'';offset=datastart
 for chunk,blob in zip(chunks,blobs):
  packed+=be(4 if name=='none' else 0,1)+be(len(blob),3)+be(offset,6)+be(binascii.crc_hqx(chunk,0xffff),2);offset+=len(blob)
 maphead=be(len(mapbody),4)+be(datastart,6)+be(binascii.crc_hqx(packed,0xffff),2)+bytes([24,0,0,0])
 codec=b'zlib' if name=='none' else name.encode()
 header=b'MComprHD'+be(124,4)+be(5,4)+codec+bytes(12)+be(len(raw),8)+be(mo,8)+be(metaoff,8)+be(hb,4)+be(2448,4)+bytes(60)
 img=header+maphead+mapbody+b'CHT2'+be(len(meta),4)+bytes(8)+meta+b''.join(blobs)
 (out/(name+'.chd')).write_bytes(img)
# V5 uncompressed map uses big-endian physical-hunk indices.
metaoff=124+4*len(chunks)
header=b'MComprHD'+be(124,4)+be(5,4)+bytes(16)+be(len(raw),8)+be(124,8)+be(metaoff,8)+be(hb,4)+be(2448,4)+bytes(60)
image=header+b''.join(be(i+1,4) for i in range(len(chunks)))+b'CHT2'+be(len(meta),4)+bytes(8)+meta
image+=bytes(hb-len(image))+raw
(out/'uncompressed.chd').write_bytes(image)
for ver,hl in [(3,120),(4,108)]:
 metaoff=hl+16*5;off=metaoff+16+len(meta);maps=b'';blobs=[]
 for chunk in chunks:
  blob=rawzip(chunk);maps+=be(off,8)+be(zlib.crc32(chunk),4)+be(len(blob)&0xffff,2)+bytes([len(blob)>>16,1]);blobs.append(blob);off+=len(blob)
 h=bytearray(hl);h[:8]=b'MComprHD';h[8:16]=be(hl,4)+be(ver,4);h[20:44]=be(1,4)+be(4,4)+be(len(raw),8)+be(metaoff,8);idx=76 if ver==3 else 44;h[idx:idx+4]=be(hb,4)
 (out/('v'+str(ver)+'.chd')).write_bytes(h+maps+b'EndOfListCookie\0'+b'CHTR'+be(len(meta),4)+bytes(8)+meta+b''.join(blobs))
with zipfile.ZipFile(out/'roms.zip','w') as za:
 data=bytes(((i*13)^(i>>7))&255 for i in range(2000000))
 za.writestr('stored.bin',data,compress_type=zipfile.ZIP_STORED)
 za.writestr('deflate.bin',data,compress_type=zipfile.ZIP_DEFLATED)
 za.writestr('empty.bin',b'',compress_type=zipfile.ZIP_DEFLATED)
 seed=12345;random=bytearray()
 for _ in range(300000):
  seed^=(seed<<13)&0xffffffff;seed^=seed>>17;seed^=(seed<<5)&0xffffffff
  random.append(seed&255)
 za.writestr('random.bin',random,compress_type=zipfile.ZIP_DEFLATED)
 assert za.getinfo('random.bin').compress_size>65536
print('Generated',len(list(out.glob('*.chd'))),'CHD fixtures and ZIP')
