case 0x0000:
case 0x0001:
case 0x0002:
case 0x0003:
case 0x0004:
case 0x0005:
case 0x0006:
case 0x0007:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0010:
case 0x0011:
case 0x0012:
case 0x0013:
case 0x0014:
case 0x0015:
case 0x0016:
case 0x0017:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0018:
case 0x0019:
case 0x001a:
case 0x001b:
case 0x001c:
case 0x001d:
case 0x001e:
case 0x001f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0020:
case 0x0021:
case 0x0022:
case 0x0023:
case 0x0024:
case 0x0025:
case 0x0026:
case 0x0027:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0028:
case 0x0029:
case 0x002a:
case 0x002b:
case 0x002c:
case 0x002d:
case 0x002e:
case 0x002f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0030:
case 0x0031:
case 0x0032:
case 0x0033:
case 0x0034:
case 0x0035:
case 0x0036:
case 0x0037:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0038:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0039:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x003a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x003b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x003c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0040:
case 0x0041:
case 0x0042:
case 0x0043:
case 0x0044:
case 0x0045:
case 0x0046:
case 0x0047:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0050:
case 0x0051:
case 0x0052:
case 0x0053:
case 0x0054:
case 0x0055:
case 0x0056:
case 0x0057:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0058:
case 0x0059:
case 0x005a:
case 0x005b:
case 0x005c:
case 0x005d:
case 0x005e:
case 0x005f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0060:
case 0x0061:
case 0x0062:
case 0x0063:
case 0x0064:
case 0x0065:
case 0x0066:
case 0x0067:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0068:
case 0x0069:
case 0x006a:
case 0x006b:
case 0x006c:
case 0x006d:
case 0x006e:
case 0x006f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0070:
case 0x0071:
case 0x0072:
case 0x0073:
case 0x0074:
case 0x0075:
case 0x0076:
case 0x0077:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0078:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0079:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x007a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x007b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x007c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0080:
case 0x0081:
case 0x0082:
case 0x0083:
case 0x0084:
case 0x0085:
case 0x0086:
case 0x0087:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0090:
case 0x0091:
case 0x0092:
case 0x0093:
case 0x0094:
case 0x0095:
case 0x0096:
case 0x0097:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0098:
case 0x0099:
case 0x009a:
case 0x009b:
case 0x009c:
case 0x009d:
case 0x009e:
case 0x009f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00a0:
case 0x00a1:
case 0x00a2:
case 0x00a3:
case 0x00a4:
case 0x00a5:
case 0x00a6:
case 0x00a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00a8:
case 0x00a9:
case 0x00aa:
case 0x00ab:
case 0x00ac:
case 0x00ad:
case 0x00ae:
case 0x00af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00b0:
case 0x00b1:
case 0x00b2:
case 0x00b3:
case 0x00b4:
case 0x00b5:
case 0x00b6:
case 0x00b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x00c0:
case 0x00c1:
case 0x00c2:
case 0x00c3:
case 0x00c4:
case 0x00c5:
case 0x00c6:
case 0x00c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00d0:
case 0x00d1:
case 0x00d2:
case 0x00d3:
case 0x00d4:
case 0x00d5:
case 0x00d6:
case 0x00d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00d8:
case 0x00d9:
case 0x00da:
case 0x00db:
case 0x00dc:
case 0x00dd:
case 0x00de:
case 0x00df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00e0:
case 0x00e1:
case 0x00e2:
case 0x00e3:
case 0x00e4:
case 0x00e5:
case 0x00e6:
case 0x00e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00e8:
case 0x00e9:
case 0x00ea:
case 0x00eb:
case 0x00ec:
case 0x00ed:
case 0x00ee:
case 0x00ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00f0:
case 0x00f1:
case 0x00f2:
case 0x00f3:
case 0x00f4:
case 0x00f5:
case 0x00f6:
case 0x00f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x00fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0100:
case 0x0101:
case 0x0102:
case 0x0103:
case 0x0104:
case 0x0105:
case 0x0106:
case 0x0107:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0108:
case 0x0109:
case 0x010a:
case 0x010b:
case 0x010c:
case 0x010d:
case 0x010e:
case 0x010f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0110:
case 0x0111:
case 0x0112:
case 0x0113:
case 0x0114:
case 0x0115:
case 0x0116:
case 0x0117:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0118:
case 0x0119:
case 0x011a:
case 0x011b:
case 0x011c:
case 0x011d:
case 0x011e:
case 0x011f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0120:
case 0x0121:
case 0x0122:
case 0x0123:
case 0x0124:
case 0x0125:
case 0x0126:
case 0x0127:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0128:
case 0x0129:
case 0x012a:
case 0x012b:
case 0x012c:
case 0x012d:
case 0x012e:
case 0x012f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0130:
case 0x0131:
case 0x0132:
case 0x0133:
case 0x0134:
case 0x0135:
case 0x0136:
case 0x0137:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0138:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0139:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0150:
case 0x0151:
case 0x0152:
case 0x0153:
case 0x0154:
case 0x0155:
case 0x0156:
case 0x0157:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0158:
case 0x0159:
case 0x015a:
case 0x015b:
case 0x015c:
case 0x015d:
case 0x015e:
case 0x015f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0160:
case 0x0161:
case 0x0162:
case 0x0163:
case 0x0164:
case 0x0165:
case 0x0166:
case 0x0167:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0168:
case 0x0169:
case 0x016a:
case 0x016b:
case 0x016c:
case 0x016d:
case 0x016e:
case 0x016f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0170:
case 0x0171:
case 0x0172:
case 0x0173:
case 0x0174:
case 0x0175:
case 0x0176:
case 0x0177:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0178:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0179:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0190:
case 0x0191:
case 0x0192:
case 0x0193:
case 0x0194:
case 0x0195:
case 0x0196:
case 0x0197:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0198:
case 0x0199:
case 0x019a:
case 0x019b:
case 0x019c:
case 0x019d:
case 0x019e:
case 0x019f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x01a0:
case 0x01a1:
case 0x01a2:
case 0x01a3:
case 0x01a4:
case 0x01a5:
case 0x01a6:
case 0x01a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x01a8:
case 0x01a9:
case 0x01aa:
case 0x01ab:
case 0x01ac:
case 0x01ad:
case 0x01ae:
case 0x01af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x01b0:
case 0x01b1:
case 0x01b2:
case 0x01b3:
case 0x01b4:
case 0x01b5:
case 0x01b6:
case 0x01b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x01b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x01b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x01c0:
case 0x01c1:
case 0x01c2:
case 0x01c3:
case 0x01c4:
case 0x01c5:
case 0x01c6:
case 0x01c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01d0:
case 0x01d1:
case 0x01d2:
case 0x01d3:
case 0x01d4:
case 0x01d5:
case 0x01d6:
case 0x01d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01d8:
case 0x01d9:
case 0x01da:
case 0x01db:
case 0x01dc:
case 0x01dd:
case 0x01de:
case 0x01df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01e0:
case 0x01e1:
case 0x01e2:
case 0x01e3:
case 0x01e4:
case 0x01e5:
case 0x01e6:
case 0x01e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01e8:
case 0x01e9:
case 0x01ea:
case 0x01eb:
case 0x01ec:
case 0x01ed:
case 0x01ee:
case 0x01ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01f0:
case 0x01f1:
case 0x01f2:
case 0x01f3:
case 0x01f4:
case 0x01f5:
case 0x01f6:
case 0x01f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x01fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0200:
case 0x0201:
case 0x0202:
case 0x0203:
case 0x0204:
case 0x0205:
case 0x0206:
case 0x0207:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0210:
case 0x0211:
case 0x0212:
case 0x0213:
case 0x0214:
case 0x0215:
case 0x0216:
case 0x0217:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0218:
case 0x0219:
case 0x021a:
case 0x021b:
case 0x021c:
case 0x021d:
case 0x021e:
case 0x021f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0220:
case 0x0221:
case 0x0222:
case 0x0223:
case 0x0224:
case 0x0225:
case 0x0226:
case 0x0227:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0228:
case 0x0229:
case 0x022a:
case 0x022b:
case 0x022c:
case 0x022d:
case 0x022e:
case 0x022f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0230:
case 0x0231:
case 0x0232:
case 0x0233:
case 0x0234:
case 0x0235:
case 0x0236:
case 0x0237:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0238:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0239:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x023a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x023b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x023c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0240:
case 0x0241:
case 0x0242:
case 0x0243:
case 0x0244:
case 0x0245:
case 0x0246:
case 0x0247:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0250:
case 0x0251:
case 0x0252:
case 0x0253:
case 0x0254:
case 0x0255:
case 0x0256:
case 0x0257:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0258:
case 0x0259:
case 0x025a:
case 0x025b:
case 0x025c:
case 0x025d:
case 0x025e:
case 0x025f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0260:
case 0x0261:
case 0x0262:
case 0x0263:
case 0x0264:
case 0x0265:
case 0x0266:
case 0x0267:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0268:
case 0x0269:
case 0x026a:
case 0x026b:
case 0x026c:
case 0x026d:
case 0x026e:
case 0x026f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0270:
case 0x0271:
case 0x0272:
case 0x0273:
case 0x0274:
case 0x0275:
case 0x0276:
case 0x0277:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0278:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0279:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x027a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x027b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x027c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0280:
case 0x0281:
case 0x0282:
case 0x0283:
case 0x0284:
case 0x0285:
case 0x0286:
case 0x0287:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0290:
case 0x0291:
case 0x0292:
case 0x0293:
case 0x0294:
case 0x0295:
case 0x0296:
case 0x0297:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0298:
case 0x0299:
case 0x029a:
case 0x029b:
case 0x029c:
case 0x029d:
case 0x029e:
case 0x029f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02a0:
case 0x02a1:
case 0x02a2:
case 0x02a3:
case 0x02a4:
case 0x02a5:
case 0x02a6:
case 0x02a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02a8:
case 0x02a9:
case 0x02aa:
case 0x02ab:
case 0x02ac:
case 0x02ad:
case 0x02ae:
case 0x02af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02b0:
case 0x02b1:
case 0x02b2:
case 0x02b3:
case 0x02b4:
case 0x02b5:
case 0x02b6:
case 0x02b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x02c0:
case 0x02c1:
case 0x02c2:
case 0x02c3:
case 0x02c4:
case 0x02c5:
case 0x02c6:
case 0x02c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02d0:
case 0x02d1:
case 0x02d2:
case 0x02d3:
case 0x02d4:
case 0x02d5:
case 0x02d6:
case 0x02d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02d8:
case 0x02d9:
case 0x02da:
case 0x02db:
case 0x02dc:
case 0x02dd:
case 0x02de:
case 0x02df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02e0:
case 0x02e1:
case 0x02e2:
case 0x02e3:
case 0x02e4:
case 0x02e5:
case 0x02e6:
case 0x02e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02e8:
case 0x02e9:
case 0x02ea:
case 0x02eb:
case 0x02ec:
case 0x02ed:
case 0x02ee:
case 0x02ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02f0:
case 0x02f1:
case 0x02f2:
case 0x02f3:
case 0x02f4:
case 0x02f5:
case 0x02f6:
case 0x02f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x02fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0300:
case 0x0301:
case 0x0302:
case 0x0303:
case 0x0304:
case 0x0305:
case 0x0306:
case 0x0307:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0308:
case 0x0309:
case 0x030a:
case 0x030b:
case 0x030c:
case 0x030d:
case 0x030e:
case 0x030f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0310:
case 0x0311:
case 0x0312:
case 0x0313:
case 0x0314:
case 0x0315:
case 0x0316:
case 0x0317:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0318:
case 0x0319:
case 0x031a:
case 0x031b:
case 0x031c:
case 0x031d:
case 0x031e:
case 0x031f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0320:
case 0x0321:
case 0x0322:
case 0x0323:
case 0x0324:
case 0x0325:
case 0x0326:
case 0x0327:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0328:
case 0x0329:
case 0x032a:
case 0x032b:
case 0x032c:
case 0x032d:
case 0x032e:
case 0x032f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0330:
case 0x0331:
case 0x0332:
case 0x0333:
case 0x0334:
case 0x0335:
case 0x0336:
case 0x0337:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0338:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0339:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0350:
case 0x0351:
case 0x0352:
case 0x0353:
case 0x0354:
case 0x0355:
case 0x0356:
case 0x0357:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0358:
case 0x0359:
case 0x035a:
case 0x035b:
case 0x035c:
case 0x035d:
case 0x035e:
case 0x035f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0360:
case 0x0361:
case 0x0362:
case 0x0363:
case 0x0364:
case 0x0365:
case 0x0366:
case 0x0367:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0368:
case 0x0369:
case 0x036a:
case 0x036b:
case 0x036c:
case 0x036d:
case 0x036e:
case 0x036f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0370:
case 0x0371:
case 0x0372:
case 0x0373:
case 0x0374:
case 0x0375:
case 0x0376:
case 0x0377:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0378:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0379:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0390:
case 0x0391:
case 0x0392:
case 0x0393:
case 0x0394:
case 0x0395:
case 0x0396:
case 0x0397:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0398:
case 0x0399:
case 0x039a:
case 0x039b:
case 0x039c:
case 0x039d:
case 0x039e:
case 0x039f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x03a0:
case 0x03a1:
case 0x03a2:
case 0x03a3:
case 0x03a4:
case 0x03a5:
case 0x03a6:
case 0x03a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x03a8:
case 0x03a9:
case 0x03aa:
case 0x03ab:
case 0x03ac:
case 0x03ad:
case 0x03ae:
case 0x03af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x03b0:
case 0x03b1:
case 0x03b2:
case 0x03b3:
case 0x03b4:
case 0x03b5:
case 0x03b6:
case 0x03b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x03b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x03b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x03c0:
case 0x03c1:
case 0x03c2:
case 0x03c3:
case 0x03c4:
case 0x03c5:
case 0x03c6:
case 0x03c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03d0:
case 0x03d1:
case 0x03d2:
case 0x03d3:
case 0x03d4:
case 0x03d5:
case 0x03d6:
case 0x03d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03d8:
case 0x03d9:
case 0x03da:
case 0x03db:
case 0x03dc:
case 0x03dd:
case 0x03de:
case 0x03df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03e0:
case 0x03e1:
case 0x03e2:
case 0x03e3:
case 0x03e4:
case 0x03e5:
case 0x03e6:
case 0x03e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03e8:
case 0x03e9:
case 0x03ea:
case 0x03eb:
case 0x03ec:
case 0x03ed:
case 0x03ee:
case 0x03ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03f0:
case 0x03f1:
case 0x03f2:
case 0x03f3:
case 0x03f4:
case 0x03f5:
case 0x03f6:
case 0x03f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x03fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0400:
case 0x0401:
case 0x0402:
case 0x0403:
case 0x0404:
case 0x0405:
case 0x0406:
case 0x0407:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0410:
case 0x0411:
case 0x0412:
case 0x0413:
case 0x0414:
case 0x0415:
case 0x0416:
case 0x0417:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0418:
case 0x0419:
case 0x041a:
case 0x041b:
case 0x041c:
case 0x041d:
case 0x041e:
case 0x041f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0420:
case 0x0421:
case 0x0422:
case 0x0423:
case 0x0424:
case 0x0425:
case 0x0426:
case 0x0427:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0428:
case 0x0429:
case 0x042a:
case 0x042b:
case 0x042c:
case 0x042d:
case 0x042e:
case 0x042f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0430:
case 0x0431:
case 0x0432:
case 0x0433:
case 0x0434:
case 0x0435:
case 0x0436:
case 0x0437:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0438:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0439:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x043a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x043b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x043c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0440:
case 0x0441:
case 0x0442:
case 0x0443:
case 0x0444:
case 0x0445:
case 0x0446:
case 0x0447:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0450:
case 0x0451:
case 0x0452:
case 0x0453:
case 0x0454:
case 0x0455:
case 0x0456:
case 0x0457:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0458:
case 0x0459:
case 0x045a:
case 0x045b:
case 0x045c:
case 0x045d:
case 0x045e:
case 0x045f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0460:
case 0x0461:
case 0x0462:
case 0x0463:
case 0x0464:
case 0x0465:
case 0x0466:
case 0x0467:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0468:
case 0x0469:
case 0x046a:
case 0x046b:
case 0x046c:
case 0x046d:
case 0x046e:
case 0x046f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0470:
case 0x0471:
case 0x0472:
case 0x0473:
case 0x0474:
case 0x0475:
case 0x0476:
case 0x0477:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0478:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0479:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x047a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x047b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x047c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0480:
case 0x0481:
case 0x0482:
case 0x0483:
case 0x0484:
case 0x0485:
case 0x0486:
case 0x0487:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0490:
case 0x0491:
case 0x0492:
case 0x0493:
case 0x0494:
case 0x0495:
case 0x0496:
case 0x0497:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0498:
case 0x0499:
case 0x049a:
case 0x049b:
case 0x049c:
case 0x049d:
case 0x049e:
case 0x049f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04a0:
case 0x04a1:
case 0x04a2:
case 0x04a3:
case 0x04a4:
case 0x04a5:
case 0x04a6:
case 0x04a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04a8:
case 0x04a9:
case 0x04aa:
case 0x04ab:
case 0x04ac:
case 0x04ad:
case 0x04ae:
case 0x04af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04b0:
case 0x04b1:
case 0x04b2:
case 0x04b3:
case 0x04b4:
case 0x04b5:
case 0x04b6:
case 0x04b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x04c0:
case 0x04c1:
case 0x04c2:
case 0x04c3:
case 0x04c4:
case 0x04c5:
case 0x04c6:
case 0x04c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04d0:
case 0x04d1:
case 0x04d2:
case 0x04d3:
case 0x04d4:
case 0x04d5:
case 0x04d6:
case 0x04d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04d8:
case 0x04d9:
case 0x04da:
case 0x04db:
case 0x04dc:
case 0x04dd:
case 0x04de:
case 0x04df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04e0:
case 0x04e1:
case 0x04e2:
case 0x04e3:
case 0x04e4:
case 0x04e5:
case 0x04e6:
case 0x04e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04e8:
case 0x04e9:
case 0x04ea:
case 0x04eb:
case 0x04ec:
case 0x04ed:
case 0x04ee:
case 0x04ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04f0:
case 0x04f1:
case 0x04f2:
case 0x04f3:
case 0x04f4:
case 0x04f5:
case 0x04f6:
case 0x04f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x04fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0500:
case 0x0501:
case 0x0502:
case 0x0503:
case 0x0504:
case 0x0505:
case 0x0506:
case 0x0507:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0508:
case 0x0509:
case 0x050a:
case 0x050b:
case 0x050c:
case 0x050d:
case 0x050e:
case 0x050f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0510:
case 0x0511:
case 0x0512:
case 0x0513:
case 0x0514:
case 0x0515:
case 0x0516:
case 0x0517:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0518:
case 0x0519:
case 0x051a:
case 0x051b:
case 0x051c:
case 0x051d:
case 0x051e:
case 0x051f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0520:
case 0x0521:
case 0x0522:
case 0x0523:
case 0x0524:
case 0x0525:
case 0x0526:
case 0x0527:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0528:
case 0x0529:
case 0x052a:
case 0x052b:
case 0x052c:
case 0x052d:
case 0x052e:
case 0x052f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0530:
case 0x0531:
case 0x0532:
case 0x0533:
case 0x0534:
case 0x0535:
case 0x0536:
case 0x0537:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0538:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0539:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0550:
case 0x0551:
case 0x0552:
case 0x0553:
case 0x0554:
case 0x0555:
case 0x0556:
case 0x0557:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0558:
case 0x0559:
case 0x055a:
case 0x055b:
case 0x055c:
case 0x055d:
case 0x055e:
case 0x055f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0560:
case 0x0561:
case 0x0562:
case 0x0563:
case 0x0564:
case 0x0565:
case 0x0566:
case 0x0567:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0568:
case 0x0569:
case 0x056a:
case 0x056b:
case 0x056c:
case 0x056d:
case 0x056e:
case 0x056f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0570:
case 0x0571:
case 0x0572:
case 0x0573:
case 0x0574:
case 0x0575:
case 0x0576:
case 0x0577:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0578:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0579:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0590:
case 0x0591:
case 0x0592:
case 0x0593:
case 0x0594:
case 0x0595:
case 0x0596:
case 0x0597:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0598:
case 0x0599:
case 0x059a:
case 0x059b:
case 0x059c:
case 0x059d:
case 0x059e:
case 0x059f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x05a0:
case 0x05a1:
case 0x05a2:
case 0x05a3:
case 0x05a4:
case 0x05a5:
case 0x05a6:
case 0x05a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x05a8:
case 0x05a9:
case 0x05aa:
case 0x05ab:
case 0x05ac:
case 0x05ad:
case 0x05ae:
case 0x05af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x05b0:
case 0x05b1:
case 0x05b2:
case 0x05b3:
case 0x05b4:
case 0x05b5:
case 0x05b6:
case 0x05b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x05b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x05b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x05c0:
case 0x05c1:
case 0x05c2:
case 0x05c3:
case 0x05c4:
case 0x05c5:
case 0x05c6:
case 0x05c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05d0:
case 0x05d1:
case 0x05d2:
case 0x05d3:
case 0x05d4:
case 0x05d5:
case 0x05d6:
case 0x05d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05d8:
case 0x05d9:
case 0x05da:
case 0x05db:
case 0x05dc:
case 0x05dd:
case 0x05de:
case 0x05df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05e0:
case 0x05e1:
case 0x05e2:
case 0x05e3:
case 0x05e4:
case 0x05e5:
case 0x05e6:
case 0x05e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05e8:
case 0x05e9:
case 0x05ea:
case 0x05eb:
case 0x05ec:
case 0x05ed:
case 0x05ee:
case 0x05ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05f0:
case 0x05f1:
case 0x05f2:
case 0x05f3:
case 0x05f4:
case 0x05f5:
case 0x05f6:
case 0x05f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x05fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0600:
case 0x0601:
case 0x0602:
case 0x0603:
case 0x0604:
case 0x0605:
case 0x0606:
case 0x0607:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0610:
case 0x0611:
case 0x0612:
case 0x0613:
case 0x0614:
case 0x0615:
case 0x0616:
case 0x0617:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0618:
case 0x0619:
case 0x061a:
case 0x061b:
case 0x061c:
case 0x061d:
case 0x061e:
case 0x061f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0620:
case 0x0621:
case 0x0622:
case 0x0623:
case 0x0624:
case 0x0625:
case 0x0626:
case 0x0627:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0628:
case 0x0629:
case 0x062a:
case 0x062b:
case 0x062c:
case 0x062d:
case 0x062e:
case 0x062f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0630:
case 0x0631:
case 0x0632:
case 0x0633:
case 0x0634:
case 0x0635:
case 0x0636:
case 0x0637:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0638:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0639:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x063a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x063b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x063c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0640:
case 0x0641:
case 0x0642:
case 0x0643:
case 0x0644:
case 0x0645:
case 0x0646:
case 0x0647:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0650:
case 0x0651:
case 0x0652:
case 0x0653:
case 0x0654:
case 0x0655:
case 0x0656:
case 0x0657:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0658:
case 0x0659:
case 0x065a:
case 0x065b:
case 0x065c:
case 0x065d:
case 0x065e:
case 0x065f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0660:
case 0x0661:
case 0x0662:
case 0x0663:
case 0x0664:
case 0x0665:
case 0x0666:
case 0x0667:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0668:
case 0x0669:
case 0x066a:
case 0x066b:
case 0x066c:
case 0x066d:
case 0x066e:
case 0x066f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0670:
case 0x0671:
case 0x0672:
case 0x0673:
case 0x0674:
case 0x0675:
case 0x0676:
case 0x0677:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0678:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0679:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x067a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x067b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x067c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0680:
case 0x0681:
case 0x0682:
case 0x0683:
case 0x0684:
case 0x0685:
case 0x0686:
case 0x0687:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0690:
case 0x0691:
case 0x0692:
case 0x0693:
case 0x0694:
case 0x0695:
case 0x0696:
case 0x0697:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0698:
case 0x0699:
case 0x069a:
case 0x069b:
case 0x069c:
case 0x069d:
case 0x069e:
case 0x069f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06a0:
case 0x06a1:
case 0x06a2:
case 0x06a3:
case 0x06a4:
case 0x06a5:
case 0x06a6:
case 0x06a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06a8:
case 0x06a9:
case 0x06aa:
case 0x06ab:
case 0x06ac:
case 0x06ad:
case 0x06ae:
case 0x06af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06b0:
case 0x06b1:
case 0x06b2:
case 0x06b3:
case 0x06b4:
case 0x06b5:
case 0x06b6:
case 0x06b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x06c0:
case 0x06c1:
case 0x06c2:
case 0x06c3:
case 0x06c4:
case 0x06c5:
case 0x06c6:
case 0x06c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06d0:
case 0x06d1:
case 0x06d2:
case 0x06d3:
case 0x06d4:
case 0x06d5:
case 0x06d6:
case 0x06d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06d8:
case 0x06d9:
case 0x06da:
case 0x06db:
case 0x06dc:
case 0x06dd:
case 0x06de:
case 0x06df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06e0:
case 0x06e1:
case 0x06e2:
case 0x06e3:
case 0x06e4:
case 0x06e5:
case 0x06e6:
case 0x06e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06e8:
case 0x06e9:
case 0x06ea:
case 0x06eb:
case 0x06ec:
case 0x06ed:
case 0x06ee:
case 0x06ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06f0:
case 0x06f1:
case 0x06f2:
case 0x06f3:
case 0x06f4:
case 0x06f5:
case 0x06f6:
case 0x06f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x06fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0700:
case 0x0701:
case 0x0702:
case 0x0703:
case 0x0704:
case 0x0705:
case 0x0706:
case 0x0707:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0708:
case 0x0709:
case 0x070a:
case 0x070b:
case 0x070c:
case 0x070d:
case 0x070e:
case 0x070f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0710:
case 0x0711:
case 0x0712:
case 0x0713:
case 0x0714:
case 0x0715:
case 0x0716:
case 0x0717:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0718:
case 0x0719:
case 0x071a:
case 0x071b:
case 0x071c:
case 0x071d:
case 0x071e:
case 0x071f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0720:
case 0x0721:
case 0x0722:
case 0x0723:
case 0x0724:
case 0x0725:
case 0x0726:
case 0x0727:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0728:
case 0x0729:
case 0x072a:
case 0x072b:
case 0x072c:
case 0x072d:
case 0x072e:
case 0x072f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0730:
case 0x0731:
case 0x0732:
case 0x0733:
case 0x0734:
case 0x0735:
case 0x0736:
case 0x0737:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0738:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0739:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0750:
case 0x0751:
case 0x0752:
case 0x0753:
case 0x0754:
case 0x0755:
case 0x0756:
case 0x0757:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0758:
case 0x0759:
case 0x075a:
case 0x075b:
case 0x075c:
case 0x075d:
case 0x075e:
case 0x075f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0760:
case 0x0761:
case 0x0762:
case 0x0763:
case 0x0764:
case 0x0765:
case 0x0766:
case 0x0767:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0768:
case 0x0769:
case 0x076a:
case 0x076b:
case 0x076c:
case 0x076d:
case 0x076e:
case 0x076f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0770:
case 0x0771:
case 0x0772:
case 0x0773:
case 0x0774:
case 0x0775:
case 0x0776:
case 0x0777:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0778:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0779:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0790:
case 0x0791:
case 0x0792:
case 0x0793:
case 0x0794:
case 0x0795:
case 0x0796:
case 0x0797:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0798:
case 0x0799:
case 0x079a:
case 0x079b:
case 0x079c:
case 0x079d:
case 0x079e:
case 0x079f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x07a0:
case 0x07a1:
case 0x07a2:
case 0x07a3:
case 0x07a4:
case 0x07a5:
case 0x07a6:
case 0x07a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x07a8:
case 0x07a9:
case 0x07aa:
case 0x07ab:
case 0x07ac:
case 0x07ad:
case 0x07ae:
case 0x07af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x07b0:
case 0x07b1:
case 0x07b2:
case 0x07b3:
case 0x07b4:
case 0x07b5:
case 0x07b6:
case 0x07b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x07b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x07b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x07c0:
case 0x07c1:
case 0x07c2:
case 0x07c3:
case 0x07c4:
case 0x07c5:
case 0x07c6:
case 0x07c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07d0:
case 0x07d1:
case 0x07d2:
case 0x07d3:
case 0x07d4:
case 0x07d5:
case 0x07d6:
case 0x07d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07d8:
case 0x07d9:
case 0x07da:
case 0x07db:
case 0x07dc:
case 0x07dd:
case 0x07de:
case 0x07df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07e0:
case 0x07e1:
case 0x07e2:
case 0x07e3:
case 0x07e4:
case 0x07e5:
case 0x07e6:
case 0x07e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07e8:
case 0x07e9:
case 0x07ea:
case 0x07eb:
case 0x07ec:
case 0x07ed:
case 0x07ee:
case 0x07ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07f0:
case 0x07f1:
case 0x07f2:
case 0x07f3:
case 0x07f4:
case 0x07f5:
case 0x07f6:
case 0x07f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x07fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0800:
case 0x0801:
case 0x0802:
case 0x0803:
case 0x0804:
case 0x0805:
case 0x0806:
case 0x0807:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0810:
case 0x0811:
case 0x0812:
case 0x0813:
case 0x0814:
case 0x0815:
case 0x0816:
case 0x0817:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0818:
case 0x0819:
case 0x081a:
case 0x081b:
case 0x081c:
case 0x081d:
case 0x081e:
case 0x081f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0820:
case 0x0821:
case 0x0822:
case 0x0823:
case 0x0824:
case 0x0825:
case 0x0826:
case 0x0827:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0828:
case 0x0829:
case 0x082a:
case 0x082b:
case 0x082c:
case 0x082d:
case 0x082e:
case 0x082f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0830:
case 0x0831:
case 0x0832:
case 0x0833:
case 0x0834:
case 0x0835:
case 0x0836:
case 0x0837:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0838:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0839:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x083a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x083b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x083c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0840:
case 0x0841:
case 0x0842:
case 0x0843:
case 0x0844:
case 0x0845:
case 0x0846:
case 0x0847:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0850:
case 0x0851:
case 0x0852:
case 0x0853:
case 0x0854:
case 0x0855:
case 0x0856:
case 0x0857:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0858:
case 0x0859:
case 0x085a:
case 0x085b:
case 0x085c:
case 0x085d:
case 0x085e:
case 0x085f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0860:
case 0x0861:
case 0x0862:
case 0x0863:
case 0x0864:
case 0x0865:
case 0x0866:
case 0x0867:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0868:
case 0x0869:
case 0x086a:
case 0x086b:
case 0x086c:
case 0x086d:
case 0x086e:
case 0x086f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0870:
case 0x0871:
case 0x0872:
case 0x0873:
case 0x0874:
case 0x0875:
case 0x0876:
case 0x0877:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0878:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0879:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x087a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x087b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x087c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0880:
case 0x0881:
case 0x0882:
case 0x0883:
case 0x0884:
case 0x0885:
case 0x0886:
case 0x0887:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0890:
case 0x0891:
case 0x0892:
case 0x0893:
case 0x0894:
case 0x0895:
case 0x0896:
case 0x0897:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0898:
case 0x0899:
case 0x089a:
case 0x089b:
case 0x089c:
case 0x089d:
case 0x089e:
case 0x089f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08a0:
case 0x08a1:
case 0x08a2:
case 0x08a3:
case 0x08a4:
case 0x08a5:
case 0x08a6:
case 0x08a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08a8:
case 0x08a9:
case 0x08aa:
case 0x08ab:
case 0x08ac:
case 0x08ad:
case 0x08ae:
case 0x08af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08b0:
case 0x08b1:
case 0x08b2:
case 0x08b3:
case 0x08b4:
case 0x08b5:
case 0x08b6:
case 0x08b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x08c0:
case 0x08c1:
case 0x08c2:
case 0x08c3:
case 0x08c4:
case 0x08c5:
case 0x08c6:
case 0x08c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08d0:
case 0x08d1:
case 0x08d2:
case 0x08d3:
case 0x08d4:
case 0x08d5:
case 0x08d6:
case 0x08d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08d8:
case 0x08d9:
case 0x08da:
case 0x08db:
case 0x08dc:
case 0x08dd:
case 0x08de:
case 0x08df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08e0:
case 0x08e1:
case 0x08e2:
case 0x08e3:
case 0x08e4:
case 0x08e5:
case 0x08e6:
case 0x08e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08e8:
case 0x08e9:
case 0x08ea:
case 0x08eb:
case 0x08ec:
case 0x08ed:
case 0x08ee:
case 0x08ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08f0:
case 0x08f1:
case 0x08f2:
case 0x08f3:
case 0x08f4:
case 0x08f5:
case 0x08f6:
case 0x08f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x08fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0900:
case 0x0901:
case 0x0902:
case 0x0903:
case 0x0904:
case 0x0905:
case 0x0906:
case 0x0907:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0908:
case 0x0909:
case 0x090a:
case 0x090b:
case 0x090c:
case 0x090d:
case 0x090e:
case 0x090f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0910:
case 0x0911:
case 0x0912:
case 0x0913:
case 0x0914:
case 0x0915:
case 0x0916:
case 0x0917:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0918:
case 0x0919:
case 0x091a:
case 0x091b:
case 0x091c:
case 0x091d:
case 0x091e:
case 0x091f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0920:
case 0x0921:
case 0x0922:
case 0x0923:
case 0x0924:
case 0x0925:
case 0x0926:
case 0x0927:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0928:
case 0x0929:
case 0x092a:
case 0x092b:
case 0x092c:
case 0x092d:
case 0x092e:
case 0x092f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0930:
case 0x0931:
case 0x0932:
case 0x0933:
case 0x0934:
case 0x0935:
case 0x0936:
case 0x0937:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0938:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0939:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0950:
case 0x0951:
case 0x0952:
case 0x0953:
case 0x0954:
case 0x0955:
case 0x0956:
case 0x0957:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0958:
case 0x0959:
case 0x095a:
case 0x095b:
case 0x095c:
case 0x095d:
case 0x095e:
case 0x095f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0960:
case 0x0961:
case 0x0962:
case 0x0963:
case 0x0964:
case 0x0965:
case 0x0966:
case 0x0967:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0968:
case 0x0969:
case 0x096a:
case 0x096b:
case 0x096c:
case 0x096d:
case 0x096e:
case 0x096f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0970:
case 0x0971:
case 0x0972:
case 0x0973:
case 0x0974:
case 0x0975:
case 0x0976:
case 0x0977:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0978:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0979:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0990:
case 0x0991:
case 0x0992:
case 0x0993:
case 0x0994:
case 0x0995:
case 0x0996:
case 0x0997:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0998:
case 0x0999:
case 0x099a:
case 0x099b:
case 0x099c:
case 0x099d:
case 0x099e:
case 0x099f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x09a0:
case 0x09a1:
case 0x09a2:
case 0x09a3:
case 0x09a4:
case 0x09a5:
case 0x09a6:
case 0x09a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x09a8:
case 0x09a9:
case 0x09aa:
case 0x09ab:
case 0x09ac:
case 0x09ad:
case 0x09ae:
case 0x09af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x09b0:
case 0x09b1:
case 0x09b2:
case 0x09b3:
case 0x09b4:
case 0x09b5:
case 0x09b6:
case 0x09b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x09b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x09b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x09c0:
case 0x09c1:
case 0x09c2:
case 0x09c3:
case 0x09c4:
case 0x09c5:
case 0x09c6:
case 0x09c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09d0:
case 0x09d1:
case 0x09d2:
case 0x09d3:
case 0x09d4:
case 0x09d5:
case 0x09d6:
case 0x09d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09d8:
case 0x09d9:
case 0x09da:
case 0x09db:
case 0x09dc:
case 0x09dd:
case 0x09de:
case 0x09df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09e0:
case 0x09e1:
case 0x09e2:
case 0x09e3:
case 0x09e4:
case 0x09e5:
case 0x09e6:
case 0x09e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09e8:
case 0x09e9:
case 0x09ea:
case 0x09eb:
case 0x09ec:
case 0x09ed:
case 0x09ee:
case 0x09ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09f0:
case 0x09f1:
case 0x09f2:
case 0x09f3:
case 0x09f4:
case 0x09f5:
case 0x09f6:
case 0x09f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09f9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09fa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09fb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x09fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0a00:
case 0x0a01:
case 0x0a02:
case 0x0a03:
case 0x0a04:
case 0x0a05:
case 0x0a06:
case 0x0a07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a10:
case 0x0a11:
case 0x0a12:
case 0x0a13:
case 0x0a14:
case 0x0a15:
case 0x0a16:
case 0x0a17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a18:
case 0x0a19:
case 0x0a1a:
case 0x0a1b:
case 0x0a1c:
case 0x0a1d:
case 0x0a1e:
case 0x0a1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a20:
case 0x0a21:
case 0x0a22:
case 0x0a23:
case 0x0a24:
case 0x0a25:
case 0x0a26:
case 0x0a27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a28:
case 0x0a29:
case 0x0a2a:
case 0x0a2b:
case 0x0a2c:
case 0x0a2d:
case 0x0a2e:
case 0x0a2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a30:
case 0x0a31:
case 0x0a32:
case 0x0a33:
case 0x0a34:
case 0x0a35:
case 0x0a36:
case 0x0a37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a40:
case 0x0a41:
case 0x0a42:
case 0x0a43:
case 0x0a44:
case 0x0a45:
case 0x0a46:
case 0x0a47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a50:
case 0x0a51:
case 0x0a52:
case 0x0a53:
case 0x0a54:
case 0x0a55:
case 0x0a56:
case 0x0a57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a58:
case 0x0a59:
case 0x0a5a:
case 0x0a5b:
case 0x0a5c:
case 0x0a5d:
case 0x0a5e:
case 0x0a5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a60:
case 0x0a61:
case 0x0a62:
case 0x0a63:
case 0x0a64:
case 0x0a65:
case 0x0a66:
case 0x0a67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a68:
case 0x0a69:
case 0x0a6a:
case 0x0a6b:
case 0x0a6c:
case 0x0a6d:
case 0x0a6e:
case 0x0a6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a70:
case 0x0a71:
case 0x0a72:
case 0x0a73:
case 0x0a74:
case 0x0a75:
case 0x0a76:
case 0x0a77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a80:
case 0x0a81:
case 0x0a82:
case 0x0a83:
case 0x0a84:
case 0x0a85:
case 0x0a86:
case 0x0a87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a90:
case 0x0a91:
case 0x0a92:
case 0x0a93:
case 0x0a94:
case 0x0a95:
case 0x0a96:
case 0x0a97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0a98:
case 0x0a99:
case 0x0a9a:
case 0x0a9b:
case 0x0a9c:
case 0x0a9d:
case 0x0a9e:
case 0x0a9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0aa0:
case 0x0aa1:
case 0x0aa2:
case 0x0aa3:
case 0x0aa4:
case 0x0aa5:
case 0x0aa6:
case 0x0aa7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0aa8:
case 0x0aa9:
case 0x0aaa:
case 0x0aab:
case 0x0aac:
case 0x0aad:
case 0x0aae:
case 0x0aaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ab0:
case 0x0ab1:
case 0x0ab2:
case 0x0ab3:
case 0x0ab4:
case 0x0ab5:
case 0x0ab6:
case 0x0ab7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ab8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ab9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0aba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0abb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0abc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ac0:
case 0x0ac1:
case 0x0ac2:
case 0x0ac3:
case 0x0ac4:
case 0x0ac5:
case 0x0ac6:
case 0x0ac7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ad0:
case 0x0ad1:
case 0x0ad2:
case 0x0ad3:
case 0x0ad4:
case 0x0ad5:
case 0x0ad6:
case 0x0ad7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ad8:
case 0x0ad9:
case 0x0ada:
case 0x0adb:
case 0x0adc:
case 0x0add:
case 0x0ade:
case 0x0adf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ae0:
case 0x0ae1:
case 0x0ae2:
case 0x0ae3:
case 0x0ae4:
case 0x0ae5:
case 0x0ae6:
case 0x0ae7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ae8:
case 0x0ae9:
case 0x0aea:
case 0x0aeb:
case 0x0aec:
case 0x0aed:
case 0x0aee:
case 0x0aef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0af0:
case 0x0af1:
case 0x0af2:
case 0x0af3:
case 0x0af4:
case 0x0af5:
case 0x0af6:
case 0x0af7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0af8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0af9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0afa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0afb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0afc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0b00:
case 0x0b01:
case 0x0b02:
case 0x0b03:
case 0x0b04:
case 0x0b05:
case 0x0b06:
case 0x0b07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0b08:
case 0x0b09:
case 0x0b0a:
case 0x0b0b:
case 0x0b0c:
case 0x0b0d:
case 0x0b0e:
case 0x0b0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0b10:
case 0x0b11:
case 0x0b12:
case 0x0b13:
case 0x0b14:
case 0x0b15:
case 0x0b16:
case 0x0b17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b18:
case 0x0b19:
case 0x0b1a:
case 0x0b1b:
case 0x0b1c:
case 0x0b1d:
case 0x0b1e:
case 0x0b1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b20:
case 0x0b21:
case 0x0b22:
case 0x0b23:
case 0x0b24:
case 0x0b25:
case 0x0b26:
case 0x0b27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b28:
case 0x0b29:
case 0x0b2a:
case 0x0b2b:
case 0x0b2c:
case 0x0b2d:
case 0x0b2e:
case 0x0b2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b30:
case 0x0b31:
case 0x0b32:
case 0x0b33:
case 0x0b34:
case 0x0b35:
case 0x0b36:
case 0x0b37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0b39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0b50:
case 0x0b51:
case 0x0b52:
case 0x0b53:
case 0x0b54:
case 0x0b55:
case 0x0b56:
case 0x0b57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b58:
case 0x0b59:
case 0x0b5a:
case 0x0b5b:
case 0x0b5c:
case 0x0b5d:
case 0x0b5e:
case 0x0b5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b60:
case 0x0b61:
case 0x0b62:
case 0x0b63:
case 0x0b64:
case 0x0b65:
case 0x0b66:
case 0x0b67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b68:
case 0x0b69:
case 0x0b6a:
case 0x0b6b:
case 0x0b6c:
case 0x0b6d:
case 0x0b6e:
case 0x0b6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b70:
case 0x0b71:
case 0x0b72:
case 0x0b73:
case 0x0b74:
case 0x0b75:
case 0x0b76:
case 0x0b77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0b79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0b90:
case 0x0b91:
case 0x0b92:
case 0x0b93:
case 0x0b94:
case 0x0b95:
case 0x0b96:
case 0x0b97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0b98:
case 0x0b99:
case 0x0b9a:
case 0x0b9b:
case 0x0b9c:
case 0x0b9d:
case 0x0b9e:
case 0x0b9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0ba0:
case 0x0ba1:
case 0x0ba2:
case 0x0ba3:
case 0x0ba4:
case 0x0ba5:
case 0x0ba6:
case 0x0ba7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0ba8:
case 0x0ba9:
case 0x0baa:
case 0x0bab:
case 0x0bac:
case 0x0bad:
case 0x0bae:
case 0x0baf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0bb0:
case 0x0bb1:
case 0x0bb2:
case 0x0bb3:
case 0x0bb4:
case 0x0bb5:
case 0x0bb6:
case 0x0bb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0bb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0bb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0bc0:
case 0x0bc1:
case 0x0bc2:
case 0x0bc3:
case 0x0bc4:
case 0x0bc5:
case 0x0bc6:
case 0x0bc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bd0:
case 0x0bd1:
case 0x0bd2:
case 0x0bd3:
case 0x0bd4:
case 0x0bd5:
case 0x0bd6:
case 0x0bd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bd8:
case 0x0bd9:
case 0x0bda:
case 0x0bdb:
case 0x0bdc:
case 0x0bdd:
case 0x0bde:
case 0x0bdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0be0:
case 0x0be1:
case 0x0be2:
case 0x0be3:
case 0x0be4:
case 0x0be5:
case 0x0be6:
case 0x0be7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0be8:
case 0x0be9:
case 0x0bea:
case 0x0beb:
case 0x0bec:
case 0x0bed:
case 0x0bee:
case 0x0bef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bf0:
case 0x0bf1:
case 0x0bf2:
case 0x0bf3:
case 0x0bf4:
case 0x0bf5:
case 0x0bf6:
case 0x0bf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bf9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bfa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bfb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0bfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0c00:
case 0x0c01:
case 0x0c02:
case 0x0c03:
case 0x0c04:
case 0x0c05:
case 0x0c06:
case 0x0c07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c10:
case 0x0c11:
case 0x0c12:
case 0x0c13:
case 0x0c14:
case 0x0c15:
case 0x0c16:
case 0x0c17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c18:
case 0x0c19:
case 0x0c1a:
case 0x0c1b:
case 0x0c1c:
case 0x0c1d:
case 0x0c1e:
case 0x0c1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c20:
case 0x0c21:
case 0x0c22:
case 0x0c23:
case 0x0c24:
case 0x0c25:
case 0x0c26:
case 0x0c27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c28:
case 0x0c29:
case 0x0c2a:
case 0x0c2b:
case 0x0c2c:
case 0x0c2d:
case 0x0c2e:
case 0x0c2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c30:
case 0x0c31:
case 0x0c32:
case 0x0c33:
case 0x0c34:
case 0x0c35:
case 0x0c36:
case 0x0c37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c40:
case 0x0c41:
case 0x0c42:
case 0x0c43:
case 0x0c44:
case 0x0c45:
case 0x0c46:
case 0x0c47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c50:
case 0x0c51:
case 0x0c52:
case 0x0c53:
case 0x0c54:
case 0x0c55:
case 0x0c56:
case 0x0c57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c58:
case 0x0c59:
case 0x0c5a:
case 0x0c5b:
case 0x0c5c:
case 0x0c5d:
case 0x0c5e:
case 0x0c5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c60:
case 0x0c61:
case 0x0c62:
case 0x0c63:
case 0x0c64:
case 0x0c65:
case 0x0c66:
case 0x0c67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c68:
case 0x0c69:
case 0x0c6a:
case 0x0c6b:
case 0x0c6c:
case 0x0c6d:
case 0x0c6e:
case 0x0c6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c70:
case 0x0c71:
case 0x0c72:
case 0x0c73:
case 0x0c74:
case 0x0c75:
case 0x0c76:
case 0x0c77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c80:
case 0x0c81:
case 0x0c82:
case 0x0c83:
case 0x0c84:
case 0x0c85:
case 0x0c86:
case 0x0c87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c90:
case 0x0c91:
case 0x0c92:
case 0x0c93:
case 0x0c94:
case 0x0c95:
case 0x0c96:
case 0x0c97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0c98:
case 0x0c99:
case 0x0c9a:
case 0x0c9b:
case 0x0c9c:
case 0x0c9d:
case 0x0c9e:
case 0x0c9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ca0:
case 0x0ca1:
case 0x0ca2:
case 0x0ca3:
case 0x0ca4:
case 0x0ca5:
case 0x0ca6:
case 0x0ca7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ca8:
case 0x0ca9:
case 0x0caa:
case 0x0cab:
case 0x0cac:
case 0x0cad:
case 0x0cae:
case 0x0caf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cb0:
case 0x0cb1:
case 0x0cb2:
case 0x0cb3:
case 0x0cb4:
case 0x0cb5:
case 0x0cb6:
case 0x0cb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cbb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cbc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0cc0:
case 0x0cc1:
case 0x0cc2:
case 0x0cc3:
case 0x0cc4:
case 0x0cc5:
case 0x0cc6:
case 0x0cc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cd0:
case 0x0cd1:
case 0x0cd2:
case 0x0cd3:
case 0x0cd4:
case 0x0cd5:
case 0x0cd6:
case 0x0cd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cd8:
case 0x0cd9:
case 0x0cda:
case 0x0cdb:
case 0x0cdc:
case 0x0cdd:
case 0x0cde:
case 0x0cdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ce0:
case 0x0ce1:
case 0x0ce2:
case 0x0ce3:
case 0x0ce4:
case 0x0ce5:
case 0x0ce6:
case 0x0ce7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ce8:
case 0x0ce9:
case 0x0cea:
case 0x0ceb:
case 0x0cec:
case 0x0ced:
case 0x0cee:
case 0x0cef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cf0:
case 0x0cf1:
case 0x0cf2:
case 0x0cf3:
case 0x0cf4:
case 0x0cf5:
case 0x0cf6:
case 0x0cf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cf9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cfa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cfb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0cfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0d00:
case 0x0d01:
case 0x0d02:
case 0x0d03:
case 0x0d04:
case 0x0d05:
case 0x0d06:
case 0x0d07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0d08:
case 0x0d09:
case 0x0d0a:
case 0x0d0b:
case 0x0d0c:
case 0x0d0d:
case 0x0d0e:
case 0x0d0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0d10:
case 0x0d11:
case 0x0d12:
case 0x0d13:
case 0x0d14:
case 0x0d15:
case 0x0d16:
case 0x0d17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d18:
case 0x0d19:
case 0x0d1a:
case 0x0d1b:
case 0x0d1c:
case 0x0d1d:
case 0x0d1e:
case 0x0d1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d20:
case 0x0d21:
case 0x0d22:
case 0x0d23:
case 0x0d24:
case 0x0d25:
case 0x0d26:
case 0x0d27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d28:
case 0x0d29:
case 0x0d2a:
case 0x0d2b:
case 0x0d2c:
case 0x0d2d:
case 0x0d2e:
case 0x0d2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d30:
case 0x0d31:
case 0x0d32:
case 0x0d33:
case 0x0d34:
case 0x0d35:
case 0x0d36:
case 0x0d37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0d39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0d50:
case 0x0d51:
case 0x0d52:
case 0x0d53:
case 0x0d54:
case 0x0d55:
case 0x0d56:
case 0x0d57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d58:
case 0x0d59:
case 0x0d5a:
case 0x0d5b:
case 0x0d5c:
case 0x0d5d:
case 0x0d5e:
case 0x0d5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d60:
case 0x0d61:
case 0x0d62:
case 0x0d63:
case 0x0d64:
case 0x0d65:
case 0x0d66:
case 0x0d67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d68:
case 0x0d69:
case 0x0d6a:
case 0x0d6b:
case 0x0d6c:
case 0x0d6d:
case 0x0d6e:
case 0x0d6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d70:
case 0x0d71:
case 0x0d72:
case 0x0d73:
case 0x0d74:
case 0x0d75:
case 0x0d76:
case 0x0d77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0d79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0d90:
case 0x0d91:
case 0x0d92:
case 0x0d93:
case 0x0d94:
case 0x0d95:
case 0x0d96:
case 0x0d97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0d98:
case 0x0d99:
case 0x0d9a:
case 0x0d9b:
case 0x0d9c:
case 0x0d9d:
case 0x0d9e:
case 0x0d9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0da0:
case 0x0da1:
case 0x0da2:
case 0x0da3:
case 0x0da4:
case 0x0da5:
case 0x0da6:
case 0x0da7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0da8:
case 0x0da9:
case 0x0daa:
case 0x0dab:
case 0x0dac:
case 0x0dad:
case 0x0dae:
case 0x0daf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0db0:
case 0x0db1:
case 0x0db2:
case 0x0db3:
case 0x0db4:
case 0x0db5:
case 0x0db6:
case 0x0db7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0db8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0db9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0dc0:
case 0x0dc1:
case 0x0dc2:
case 0x0dc3:
case 0x0dc4:
case 0x0dc5:
case 0x0dc6:
case 0x0dc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0dd0:
case 0x0dd1:
case 0x0dd2:
case 0x0dd3:
case 0x0dd4:
case 0x0dd5:
case 0x0dd6:
case 0x0dd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0dd8:
case 0x0dd9:
case 0x0dda:
case 0x0ddb:
case 0x0ddc:
case 0x0ddd:
case 0x0dde:
case 0x0ddf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0de0:
case 0x0de1:
case 0x0de2:
case 0x0de3:
case 0x0de4:
case 0x0de5:
case 0x0de6:
case 0x0de7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0de8:
case 0x0de9:
case 0x0dea:
case 0x0deb:
case 0x0dec:
case 0x0ded:
case 0x0dee:
case 0x0def:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0df0:
case 0x0df1:
case 0x0df2:
case 0x0df3:
case 0x0df4:
case 0x0df5:
case 0x0df6:
case 0x0df7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0df8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0df9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0dfa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0dfb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0dfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0e00:
case 0x0e01:
case 0x0e02:
case 0x0e03:
case 0x0e04:
case 0x0e05:
case 0x0e06:
case 0x0e07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e10:
case 0x0e11:
case 0x0e12:
case 0x0e13:
case 0x0e14:
case 0x0e15:
case 0x0e16:
case 0x0e17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e18:
case 0x0e19:
case 0x0e1a:
case 0x0e1b:
case 0x0e1c:
case 0x0e1d:
case 0x0e1e:
case 0x0e1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e20:
case 0x0e21:
case 0x0e22:
case 0x0e23:
case 0x0e24:
case 0x0e25:
case 0x0e26:
case 0x0e27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e28:
case 0x0e29:
case 0x0e2a:
case 0x0e2b:
case 0x0e2c:
case 0x0e2d:
case 0x0e2e:
case 0x0e2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e30:
case 0x0e31:
case 0x0e32:
case 0x0e33:
case 0x0e34:
case 0x0e35:
case 0x0e36:
case 0x0e37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e40:
case 0x0e41:
case 0x0e42:
case 0x0e43:
case 0x0e44:
case 0x0e45:
case 0x0e46:
case 0x0e47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e50:
case 0x0e51:
case 0x0e52:
case 0x0e53:
case 0x0e54:
case 0x0e55:
case 0x0e56:
case 0x0e57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e58:
case 0x0e59:
case 0x0e5a:
case 0x0e5b:
case 0x0e5c:
case 0x0e5d:
case 0x0e5e:
case 0x0e5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e60:
case 0x0e61:
case 0x0e62:
case 0x0e63:
case 0x0e64:
case 0x0e65:
case 0x0e66:
case 0x0e67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e68:
case 0x0e69:
case 0x0e6a:
case 0x0e6b:
case 0x0e6c:
case 0x0e6d:
case 0x0e6e:
case 0x0e6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e70:
case 0x0e71:
case 0x0e72:
case 0x0e73:
case 0x0e74:
case 0x0e75:
case 0x0e76:
case 0x0e77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e80:
case 0x0e81:
case 0x0e82:
case 0x0e83:
case 0x0e84:
case 0x0e85:
case 0x0e86:
case 0x0e87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e90:
case 0x0e91:
case 0x0e92:
case 0x0e93:
case 0x0e94:
case 0x0e95:
case 0x0e96:
case 0x0e97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0e98:
case 0x0e99:
case 0x0e9a:
case 0x0e9b:
case 0x0e9c:
case 0x0e9d:
case 0x0e9e:
case 0x0e9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ea0:
case 0x0ea1:
case 0x0ea2:
case 0x0ea3:
case 0x0ea4:
case 0x0ea5:
case 0x0ea6:
case 0x0ea7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ea8:
case 0x0ea9:
case 0x0eaa:
case 0x0eab:
case 0x0eac:
case 0x0ead:
case 0x0eae:
case 0x0eaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0eb0:
case 0x0eb1:
case 0x0eb2:
case 0x0eb3:
case 0x0eb4:
case 0x0eb5:
case 0x0eb6:
case 0x0eb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0eb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0eb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0eba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ebb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ebc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); OR(src, dst);
	}
	break;

case 0x0ec0:
case 0x0ec1:
case 0x0ec2:
case 0x0ec3:
case 0x0ec4:
case 0x0ec5:
case 0x0ec6:
case 0x0ec7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ed0:
case 0x0ed1:
case 0x0ed2:
case 0x0ed3:
case 0x0ed4:
case 0x0ed5:
case 0x0ed6:
case 0x0ed7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ed8:
case 0x0ed9:
case 0x0eda:
case 0x0edb:
case 0x0edc:
case 0x0edd:
case 0x0ede:
case 0x0edf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ee0:
case 0x0ee1:
case 0x0ee2:
case 0x0ee3:
case 0x0ee4:
case 0x0ee5:
case 0x0ee6:
case 0x0ee7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ee8:
case 0x0ee9:
case 0x0eea:
case 0x0eeb:
case 0x0eec:
case 0x0eed:
case 0x0eee:
case 0x0eef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ef0:
case 0x0ef1:
case 0x0ef2:
case 0x0ef3:
case 0x0ef4:
case 0x0ef5:
case 0x0ef6:
case 0x0ef7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ef8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0ef9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0efa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0efb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0efc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVU(src, instr_b11_b9);
	}
	break;

case 0x0f00:
case 0x0f01:
case 0x0f02:
case 0x0f03:
case 0x0f04:
case 0x0f05:
case 0x0f06:
case 0x0f07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0f08:
case 0x0f09:
case 0x0f0a:
case 0x0f0b:
case 0x0f0c:
case 0x0f0d:
case 0x0f0e:
case 0x0f0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SBCD(src, dst);
	}
	break;

case 0x0f10:
case 0x0f11:
case 0x0f12:
case 0x0f13:
case 0x0f14:
case 0x0f15:
case 0x0f16:
case 0x0f17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f18:
case 0x0f19:
case 0x0f1a:
case 0x0f1b:
case 0x0f1c:
case 0x0f1d:
case 0x0f1e:
case 0x0f1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f20:
case 0x0f21:
case 0x0f22:
case 0x0f23:
case 0x0f24:
case 0x0f25:
case 0x0f26:
case 0x0f27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f28:
case 0x0f29:
case 0x0f2a:
case 0x0f2b:
case 0x0f2c:
case 0x0f2d:
case 0x0f2e:
case 0x0f2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f30:
case 0x0f31:
case 0x0f32:
case 0x0f33:
case 0x0f34:
case 0x0f35:
case 0x0f36:
case 0x0f37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0f39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0f50:
case 0x0f51:
case 0x0f52:
case 0x0f53:
case 0x0f54:
case 0x0f55:
case 0x0f56:
case 0x0f57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f58:
case 0x0f59:
case 0x0f5a:
case 0x0f5b:
case 0x0f5c:
case 0x0f5d:
case 0x0f5e:
case 0x0f5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f60:
case 0x0f61:
case 0x0f62:
case 0x0f63:
case 0x0f64:
case 0x0f65:
case 0x0f66:
case 0x0f67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f68:
case 0x0f69:
case 0x0f6a:
case 0x0f6b:
case 0x0f6c:
case 0x0f6d:
case 0x0f6e:
case 0x0f6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f70:
case 0x0f71:
case 0x0f72:
case 0x0f73:
case 0x0f74:
case 0x0f75:
case 0x0f76:
case 0x0f77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0f79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0f90:
case 0x0f91:
case 0x0f92:
case 0x0f93:
case 0x0f94:
case 0x0f95:
case 0x0f96:
case 0x0f97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0f98:
case 0x0f99:
case 0x0f9a:
case 0x0f9b:
case 0x0f9c:
case 0x0f9d:
case 0x0f9e:
case 0x0f9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0fa0:
case 0x0fa1:
case 0x0fa2:
case 0x0fa3:
case 0x0fa4:
case 0x0fa5:
case 0x0fa6:
case 0x0fa7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0fa8:
case 0x0fa9:
case 0x0faa:
case 0x0fab:
case 0x0fac:
case 0x0fad:
case 0x0fae:
case 0x0faf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0fb0:
case 0x0fb1:
case 0x0fb2:
case 0x0fb3:
case 0x0fb4:
case 0x0fb5:
case 0x0fb6:
case 0x0fb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); OR(src, dst);
	}
	break;

case 0x0fb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); OR(src, dst);
	}
	break;

case 0x0fb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); OR(src, dst);
	}
	break;

case 0x0fc0:
case 0x0fc1:
case 0x0fc2:
case 0x0fc3:
case 0x0fc4:
case 0x0fc5:
case 0x0fc6:
case 0x0fc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0fd0:
case 0x0fd1:
case 0x0fd2:
case 0x0fd3:
case 0x0fd4:
case 0x0fd5:
case 0x0fd6:
case 0x0fd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0fd8:
case 0x0fd9:
case 0x0fda:
case 0x0fdb:
case 0x0fdc:
case 0x0fdd:
case 0x0fde:
case 0x0fdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0fe0:
case 0x0fe1:
case 0x0fe2:
case 0x0fe3:
case 0x0fe4:
case 0x0fe5:
case 0x0fe6:
case 0x0fe7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0fe8:
case 0x0fe9:
case 0x0fea:
case 0x0feb:
case 0x0fec:
case 0x0fed:
case 0x0fee:
case 0x0fef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ff0:
case 0x0ff1:
case 0x0ff2:
case 0x0ff3:
case 0x0ff4:
case 0x0ff5:
case 0x0ff6:
case 0x0ff7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ff8:
	{
	 HAM<uint16, ABS_SHORT> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ff9:
	{
	 HAM<uint16, ABS_LONG> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ffa:
	{
	 HAM<uint16, PC_DISP> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ffb:
	{
	 HAM<uint16, PC_INDEX> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x0ffc:
	{
	 HAM<uint16, IMMEDIATE> src(this); DIVS(src, instr_b11_b9);
	}
	break;

case 0x1000:
case 0x1001:
case 0x1002:
case 0x1003:
case 0x1004:
case 0x1005:
case 0x1006:
case 0x1007:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1010:
case 0x1011:
case 0x1012:
case 0x1013:
case 0x1014:
case 0x1015:
case 0x1016:
case 0x1017:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1018:
case 0x1019:
case 0x101a:
case 0x101b:
case 0x101c:
case 0x101d:
case 0x101e:
case 0x101f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1020:
case 0x1021:
case 0x1022:
case 0x1023:
case 0x1024:
case 0x1025:
case 0x1026:
case 0x1027:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1028:
case 0x1029:
case 0x102a:
case 0x102b:
case 0x102c:
case 0x102d:
case 0x102e:
case 0x102f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1030:
case 0x1031:
case 0x1032:
case 0x1033:
case 0x1034:
case 0x1035:
case 0x1036:
case 0x1037:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1038:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1039:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x103a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x103b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x103c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1040:
case 0x1041:
case 0x1042:
case 0x1043:
case 0x1044:
case 0x1045:
case 0x1046:
case 0x1047:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1048:
case 0x1049:
case 0x104a:
case 0x104b:
case 0x104c:
case 0x104d:
case 0x104e:
case 0x104f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1050:
case 0x1051:
case 0x1052:
case 0x1053:
case 0x1054:
case 0x1055:
case 0x1056:
case 0x1057:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1058:
case 0x1059:
case 0x105a:
case 0x105b:
case 0x105c:
case 0x105d:
case 0x105e:
case 0x105f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1060:
case 0x1061:
case 0x1062:
case 0x1063:
case 0x1064:
case 0x1065:
case 0x1066:
case 0x1067:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1068:
case 0x1069:
case 0x106a:
case 0x106b:
case 0x106c:
case 0x106d:
case 0x106e:
case 0x106f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1070:
case 0x1071:
case 0x1072:
case 0x1073:
case 0x1074:
case 0x1075:
case 0x1076:
case 0x1077:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1078:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1079:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x107a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x107b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x107c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1080:
case 0x1081:
case 0x1082:
case 0x1083:
case 0x1084:
case 0x1085:
case 0x1086:
case 0x1087:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1088:
case 0x1089:
case 0x108a:
case 0x108b:
case 0x108c:
case 0x108d:
case 0x108e:
case 0x108f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1090:
case 0x1091:
case 0x1092:
case 0x1093:
case 0x1094:
case 0x1095:
case 0x1096:
case 0x1097:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1098:
case 0x1099:
case 0x109a:
case 0x109b:
case 0x109c:
case 0x109d:
case 0x109e:
case 0x109f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10a0:
case 0x10a1:
case 0x10a2:
case 0x10a3:
case 0x10a4:
case 0x10a5:
case 0x10a6:
case 0x10a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10a8:
case 0x10a9:
case 0x10aa:
case 0x10ab:
case 0x10ac:
case 0x10ad:
case 0x10ae:
case 0x10af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10b0:
case 0x10b1:
case 0x10b2:
case 0x10b3:
case 0x10b4:
case 0x10b5:
case 0x10b6:
case 0x10b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10c0:
case 0x10c1:
case 0x10c2:
case 0x10c3:
case 0x10c4:
case 0x10c5:
case 0x10c6:
case 0x10c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10c8:
case 0x10c9:
case 0x10ca:
case 0x10cb:
case 0x10cc:
case 0x10cd:
case 0x10ce:
case 0x10cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10d0:
case 0x10d1:
case 0x10d2:
case 0x10d3:
case 0x10d4:
case 0x10d5:
case 0x10d6:
case 0x10d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10d8:
case 0x10d9:
case 0x10da:
case 0x10db:
case 0x10dc:
case 0x10dd:
case 0x10de:
case 0x10df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10e0:
case 0x10e1:
case 0x10e2:
case 0x10e3:
case 0x10e4:
case 0x10e5:
case 0x10e6:
case 0x10e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10e8:
case 0x10e9:
case 0x10ea:
case 0x10eb:
case 0x10ec:
case 0x10ed:
case 0x10ee:
case 0x10ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10f0:
case 0x10f1:
case 0x10f2:
case 0x10f3:
case 0x10f4:
case 0x10f5:
case 0x10f6:
case 0x10f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x10fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1100:
case 0x1101:
case 0x1102:
case 0x1103:
case 0x1104:
case 0x1105:
case 0x1106:
case 0x1107:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1108:
case 0x1109:
case 0x110a:
case 0x110b:
case 0x110c:
case 0x110d:
case 0x110e:
case 0x110f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1110:
case 0x1111:
case 0x1112:
case 0x1113:
case 0x1114:
case 0x1115:
case 0x1116:
case 0x1117:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1118:
case 0x1119:
case 0x111a:
case 0x111b:
case 0x111c:
case 0x111d:
case 0x111e:
case 0x111f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1120:
case 0x1121:
case 0x1122:
case 0x1123:
case 0x1124:
case 0x1125:
case 0x1126:
case 0x1127:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1128:
case 0x1129:
case 0x112a:
case 0x112b:
case 0x112c:
case 0x112d:
case 0x112e:
case 0x112f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1130:
case 0x1131:
case 0x1132:
case 0x1133:
case 0x1134:
case 0x1135:
case 0x1136:
case 0x1137:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1138:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1139:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1140:
case 0x1141:
case 0x1142:
case 0x1143:
case 0x1144:
case 0x1145:
case 0x1146:
case 0x1147:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1148:
case 0x1149:
case 0x114a:
case 0x114b:
case 0x114c:
case 0x114d:
case 0x114e:
case 0x114f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1150:
case 0x1151:
case 0x1152:
case 0x1153:
case 0x1154:
case 0x1155:
case 0x1156:
case 0x1157:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1158:
case 0x1159:
case 0x115a:
case 0x115b:
case 0x115c:
case 0x115d:
case 0x115e:
case 0x115f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1160:
case 0x1161:
case 0x1162:
case 0x1163:
case 0x1164:
case 0x1165:
case 0x1166:
case 0x1167:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1168:
case 0x1169:
case 0x116a:
case 0x116b:
case 0x116c:
case 0x116d:
case 0x116e:
case 0x116f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1170:
case 0x1171:
case 0x1172:
case 0x1173:
case 0x1174:
case 0x1175:
case 0x1176:
case 0x1177:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1178:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1179:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1180:
case 0x1181:
case 0x1182:
case 0x1183:
case 0x1184:
case 0x1185:
case 0x1186:
case 0x1187:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1188:
case 0x1189:
case 0x118a:
case 0x118b:
case 0x118c:
case 0x118d:
case 0x118e:
case 0x118f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1190:
case 0x1191:
case 0x1192:
case 0x1193:
case 0x1194:
case 0x1195:
case 0x1196:
case 0x1197:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1198:
case 0x1199:
case 0x119a:
case 0x119b:
case 0x119c:
case 0x119d:
case 0x119e:
case 0x119f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x11a0:
case 0x11a1:
case 0x11a2:
case 0x11a3:
case 0x11a4:
case 0x11a5:
case 0x11a6:
case 0x11a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x11a8:
case 0x11a9:
case 0x11aa:
case 0x11ab:
case 0x11ac:
case 0x11ad:
case 0x11ae:
case 0x11af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x11b0:
case 0x11b1:
case 0x11b2:
case 0x11b3:
case 0x11b4:
case 0x11b5:
case 0x11b6:
case 0x11b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x11b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x11b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x11c0:
case 0x11c1:
case 0x11c2:
case 0x11c3:
case 0x11c4:
case 0x11c5:
case 0x11c6:
case 0x11c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11c8:
case 0x11c9:
case 0x11ca:
case 0x11cb:
case 0x11cc:
case 0x11cd:
case 0x11ce:
case 0x11cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11d0:
case 0x11d1:
case 0x11d2:
case 0x11d3:
case 0x11d4:
case 0x11d5:
case 0x11d6:
case 0x11d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11d8:
case 0x11d9:
case 0x11da:
case 0x11db:
case 0x11dc:
case 0x11dd:
case 0x11de:
case 0x11df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11e0:
case 0x11e1:
case 0x11e2:
case 0x11e3:
case 0x11e4:
case 0x11e5:
case 0x11e6:
case 0x11e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11e8:
case 0x11e9:
case 0x11ea:
case 0x11eb:
case 0x11ec:
case 0x11ed:
case 0x11ee:
case 0x11ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11f0:
case 0x11f1:
case 0x11f2:
case 0x11f3:
case 0x11f4:
case 0x11f5:
case 0x11f6:
case 0x11f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x11fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1200:
case 0x1201:
case 0x1202:
case 0x1203:
case 0x1204:
case 0x1205:
case 0x1206:
case 0x1207:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1210:
case 0x1211:
case 0x1212:
case 0x1213:
case 0x1214:
case 0x1215:
case 0x1216:
case 0x1217:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1218:
case 0x1219:
case 0x121a:
case 0x121b:
case 0x121c:
case 0x121d:
case 0x121e:
case 0x121f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1220:
case 0x1221:
case 0x1222:
case 0x1223:
case 0x1224:
case 0x1225:
case 0x1226:
case 0x1227:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1228:
case 0x1229:
case 0x122a:
case 0x122b:
case 0x122c:
case 0x122d:
case 0x122e:
case 0x122f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1230:
case 0x1231:
case 0x1232:
case 0x1233:
case 0x1234:
case 0x1235:
case 0x1236:
case 0x1237:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1238:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1239:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x123a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x123b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x123c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1240:
case 0x1241:
case 0x1242:
case 0x1243:
case 0x1244:
case 0x1245:
case 0x1246:
case 0x1247:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1248:
case 0x1249:
case 0x124a:
case 0x124b:
case 0x124c:
case 0x124d:
case 0x124e:
case 0x124f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1250:
case 0x1251:
case 0x1252:
case 0x1253:
case 0x1254:
case 0x1255:
case 0x1256:
case 0x1257:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1258:
case 0x1259:
case 0x125a:
case 0x125b:
case 0x125c:
case 0x125d:
case 0x125e:
case 0x125f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1260:
case 0x1261:
case 0x1262:
case 0x1263:
case 0x1264:
case 0x1265:
case 0x1266:
case 0x1267:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1268:
case 0x1269:
case 0x126a:
case 0x126b:
case 0x126c:
case 0x126d:
case 0x126e:
case 0x126f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1270:
case 0x1271:
case 0x1272:
case 0x1273:
case 0x1274:
case 0x1275:
case 0x1276:
case 0x1277:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1278:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1279:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x127a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x127b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x127c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1280:
case 0x1281:
case 0x1282:
case 0x1283:
case 0x1284:
case 0x1285:
case 0x1286:
case 0x1287:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1288:
case 0x1289:
case 0x128a:
case 0x128b:
case 0x128c:
case 0x128d:
case 0x128e:
case 0x128f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1290:
case 0x1291:
case 0x1292:
case 0x1293:
case 0x1294:
case 0x1295:
case 0x1296:
case 0x1297:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1298:
case 0x1299:
case 0x129a:
case 0x129b:
case 0x129c:
case 0x129d:
case 0x129e:
case 0x129f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12a0:
case 0x12a1:
case 0x12a2:
case 0x12a3:
case 0x12a4:
case 0x12a5:
case 0x12a6:
case 0x12a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12a8:
case 0x12a9:
case 0x12aa:
case 0x12ab:
case 0x12ac:
case 0x12ad:
case 0x12ae:
case 0x12af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12b0:
case 0x12b1:
case 0x12b2:
case 0x12b3:
case 0x12b4:
case 0x12b5:
case 0x12b6:
case 0x12b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12c0:
case 0x12c1:
case 0x12c2:
case 0x12c3:
case 0x12c4:
case 0x12c5:
case 0x12c6:
case 0x12c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12c8:
case 0x12c9:
case 0x12ca:
case 0x12cb:
case 0x12cc:
case 0x12cd:
case 0x12ce:
case 0x12cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12d0:
case 0x12d1:
case 0x12d2:
case 0x12d3:
case 0x12d4:
case 0x12d5:
case 0x12d6:
case 0x12d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12d8:
case 0x12d9:
case 0x12da:
case 0x12db:
case 0x12dc:
case 0x12dd:
case 0x12de:
case 0x12df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12e0:
case 0x12e1:
case 0x12e2:
case 0x12e3:
case 0x12e4:
case 0x12e5:
case 0x12e6:
case 0x12e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12e8:
case 0x12e9:
case 0x12ea:
case 0x12eb:
case 0x12ec:
case 0x12ed:
case 0x12ee:
case 0x12ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12f0:
case 0x12f1:
case 0x12f2:
case 0x12f3:
case 0x12f4:
case 0x12f5:
case 0x12f6:
case 0x12f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x12fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1300:
case 0x1301:
case 0x1302:
case 0x1303:
case 0x1304:
case 0x1305:
case 0x1306:
case 0x1307:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1308:
case 0x1309:
case 0x130a:
case 0x130b:
case 0x130c:
case 0x130d:
case 0x130e:
case 0x130f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1310:
case 0x1311:
case 0x1312:
case 0x1313:
case 0x1314:
case 0x1315:
case 0x1316:
case 0x1317:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1318:
case 0x1319:
case 0x131a:
case 0x131b:
case 0x131c:
case 0x131d:
case 0x131e:
case 0x131f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1320:
case 0x1321:
case 0x1322:
case 0x1323:
case 0x1324:
case 0x1325:
case 0x1326:
case 0x1327:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1328:
case 0x1329:
case 0x132a:
case 0x132b:
case 0x132c:
case 0x132d:
case 0x132e:
case 0x132f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1330:
case 0x1331:
case 0x1332:
case 0x1333:
case 0x1334:
case 0x1335:
case 0x1336:
case 0x1337:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1338:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1339:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1340:
case 0x1341:
case 0x1342:
case 0x1343:
case 0x1344:
case 0x1345:
case 0x1346:
case 0x1347:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1348:
case 0x1349:
case 0x134a:
case 0x134b:
case 0x134c:
case 0x134d:
case 0x134e:
case 0x134f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1350:
case 0x1351:
case 0x1352:
case 0x1353:
case 0x1354:
case 0x1355:
case 0x1356:
case 0x1357:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1358:
case 0x1359:
case 0x135a:
case 0x135b:
case 0x135c:
case 0x135d:
case 0x135e:
case 0x135f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1360:
case 0x1361:
case 0x1362:
case 0x1363:
case 0x1364:
case 0x1365:
case 0x1366:
case 0x1367:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1368:
case 0x1369:
case 0x136a:
case 0x136b:
case 0x136c:
case 0x136d:
case 0x136e:
case 0x136f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1370:
case 0x1371:
case 0x1372:
case 0x1373:
case 0x1374:
case 0x1375:
case 0x1376:
case 0x1377:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1378:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1379:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1380:
case 0x1381:
case 0x1382:
case 0x1383:
case 0x1384:
case 0x1385:
case 0x1386:
case 0x1387:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1388:
case 0x1389:
case 0x138a:
case 0x138b:
case 0x138c:
case 0x138d:
case 0x138e:
case 0x138f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1390:
case 0x1391:
case 0x1392:
case 0x1393:
case 0x1394:
case 0x1395:
case 0x1396:
case 0x1397:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1398:
case 0x1399:
case 0x139a:
case 0x139b:
case 0x139c:
case 0x139d:
case 0x139e:
case 0x139f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x13a0:
case 0x13a1:
case 0x13a2:
case 0x13a3:
case 0x13a4:
case 0x13a5:
case 0x13a6:
case 0x13a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x13a8:
case 0x13a9:
case 0x13aa:
case 0x13ab:
case 0x13ac:
case 0x13ad:
case 0x13ae:
case 0x13af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x13b0:
case 0x13b1:
case 0x13b2:
case 0x13b3:
case 0x13b4:
case 0x13b5:
case 0x13b6:
case 0x13b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x13b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x13b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x13c0:
case 0x13c1:
case 0x13c2:
case 0x13c3:
case 0x13c4:
case 0x13c5:
case 0x13c6:
case 0x13c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13c8:
case 0x13c9:
case 0x13ca:
case 0x13cb:
case 0x13cc:
case 0x13cd:
case 0x13ce:
case 0x13cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13d0:
case 0x13d1:
case 0x13d2:
case 0x13d3:
case 0x13d4:
case 0x13d5:
case 0x13d6:
case 0x13d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13d8:
case 0x13d9:
case 0x13da:
case 0x13db:
case 0x13dc:
case 0x13dd:
case 0x13de:
case 0x13df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13e0:
case 0x13e1:
case 0x13e2:
case 0x13e3:
case 0x13e4:
case 0x13e5:
case 0x13e6:
case 0x13e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13e8:
case 0x13e9:
case 0x13ea:
case 0x13eb:
case 0x13ec:
case 0x13ed:
case 0x13ee:
case 0x13ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13f0:
case 0x13f1:
case 0x13f2:
case 0x13f3:
case 0x13f4:
case 0x13f5:
case 0x13f6:
case 0x13f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x13fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1400:
case 0x1401:
case 0x1402:
case 0x1403:
case 0x1404:
case 0x1405:
case 0x1406:
case 0x1407:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1410:
case 0x1411:
case 0x1412:
case 0x1413:
case 0x1414:
case 0x1415:
case 0x1416:
case 0x1417:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1418:
case 0x1419:
case 0x141a:
case 0x141b:
case 0x141c:
case 0x141d:
case 0x141e:
case 0x141f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1420:
case 0x1421:
case 0x1422:
case 0x1423:
case 0x1424:
case 0x1425:
case 0x1426:
case 0x1427:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1428:
case 0x1429:
case 0x142a:
case 0x142b:
case 0x142c:
case 0x142d:
case 0x142e:
case 0x142f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1430:
case 0x1431:
case 0x1432:
case 0x1433:
case 0x1434:
case 0x1435:
case 0x1436:
case 0x1437:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1438:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1439:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x143a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x143b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x143c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1440:
case 0x1441:
case 0x1442:
case 0x1443:
case 0x1444:
case 0x1445:
case 0x1446:
case 0x1447:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1448:
case 0x1449:
case 0x144a:
case 0x144b:
case 0x144c:
case 0x144d:
case 0x144e:
case 0x144f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1450:
case 0x1451:
case 0x1452:
case 0x1453:
case 0x1454:
case 0x1455:
case 0x1456:
case 0x1457:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1458:
case 0x1459:
case 0x145a:
case 0x145b:
case 0x145c:
case 0x145d:
case 0x145e:
case 0x145f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1460:
case 0x1461:
case 0x1462:
case 0x1463:
case 0x1464:
case 0x1465:
case 0x1466:
case 0x1467:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1468:
case 0x1469:
case 0x146a:
case 0x146b:
case 0x146c:
case 0x146d:
case 0x146e:
case 0x146f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1470:
case 0x1471:
case 0x1472:
case 0x1473:
case 0x1474:
case 0x1475:
case 0x1476:
case 0x1477:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1478:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1479:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x147a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x147b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x147c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1480:
case 0x1481:
case 0x1482:
case 0x1483:
case 0x1484:
case 0x1485:
case 0x1486:
case 0x1487:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1488:
case 0x1489:
case 0x148a:
case 0x148b:
case 0x148c:
case 0x148d:
case 0x148e:
case 0x148f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1490:
case 0x1491:
case 0x1492:
case 0x1493:
case 0x1494:
case 0x1495:
case 0x1496:
case 0x1497:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1498:
case 0x1499:
case 0x149a:
case 0x149b:
case 0x149c:
case 0x149d:
case 0x149e:
case 0x149f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14a0:
case 0x14a1:
case 0x14a2:
case 0x14a3:
case 0x14a4:
case 0x14a5:
case 0x14a6:
case 0x14a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14a8:
case 0x14a9:
case 0x14aa:
case 0x14ab:
case 0x14ac:
case 0x14ad:
case 0x14ae:
case 0x14af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14b0:
case 0x14b1:
case 0x14b2:
case 0x14b3:
case 0x14b4:
case 0x14b5:
case 0x14b6:
case 0x14b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14c0:
case 0x14c1:
case 0x14c2:
case 0x14c3:
case 0x14c4:
case 0x14c5:
case 0x14c6:
case 0x14c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14c8:
case 0x14c9:
case 0x14ca:
case 0x14cb:
case 0x14cc:
case 0x14cd:
case 0x14ce:
case 0x14cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14d0:
case 0x14d1:
case 0x14d2:
case 0x14d3:
case 0x14d4:
case 0x14d5:
case 0x14d6:
case 0x14d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14d8:
case 0x14d9:
case 0x14da:
case 0x14db:
case 0x14dc:
case 0x14dd:
case 0x14de:
case 0x14df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14e0:
case 0x14e1:
case 0x14e2:
case 0x14e3:
case 0x14e4:
case 0x14e5:
case 0x14e6:
case 0x14e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14e8:
case 0x14e9:
case 0x14ea:
case 0x14eb:
case 0x14ec:
case 0x14ed:
case 0x14ee:
case 0x14ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14f0:
case 0x14f1:
case 0x14f2:
case 0x14f3:
case 0x14f4:
case 0x14f5:
case 0x14f6:
case 0x14f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x14fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1500:
case 0x1501:
case 0x1502:
case 0x1503:
case 0x1504:
case 0x1505:
case 0x1506:
case 0x1507:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1508:
case 0x1509:
case 0x150a:
case 0x150b:
case 0x150c:
case 0x150d:
case 0x150e:
case 0x150f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1510:
case 0x1511:
case 0x1512:
case 0x1513:
case 0x1514:
case 0x1515:
case 0x1516:
case 0x1517:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1518:
case 0x1519:
case 0x151a:
case 0x151b:
case 0x151c:
case 0x151d:
case 0x151e:
case 0x151f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1520:
case 0x1521:
case 0x1522:
case 0x1523:
case 0x1524:
case 0x1525:
case 0x1526:
case 0x1527:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1528:
case 0x1529:
case 0x152a:
case 0x152b:
case 0x152c:
case 0x152d:
case 0x152e:
case 0x152f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1530:
case 0x1531:
case 0x1532:
case 0x1533:
case 0x1534:
case 0x1535:
case 0x1536:
case 0x1537:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1538:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1539:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1540:
case 0x1541:
case 0x1542:
case 0x1543:
case 0x1544:
case 0x1545:
case 0x1546:
case 0x1547:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1548:
case 0x1549:
case 0x154a:
case 0x154b:
case 0x154c:
case 0x154d:
case 0x154e:
case 0x154f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1550:
case 0x1551:
case 0x1552:
case 0x1553:
case 0x1554:
case 0x1555:
case 0x1556:
case 0x1557:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1558:
case 0x1559:
case 0x155a:
case 0x155b:
case 0x155c:
case 0x155d:
case 0x155e:
case 0x155f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1560:
case 0x1561:
case 0x1562:
case 0x1563:
case 0x1564:
case 0x1565:
case 0x1566:
case 0x1567:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1568:
case 0x1569:
case 0x156a:
case 0x156b:
case 0x156c:
case 0x156d:
case 0x156e:
case 0x156f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1570:
case 0x1571:
case 0x1572:
case 0x1573:
case 0x1574:
case 0x1575:
case 0x1576:
case 0x1577:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1578:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1579:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1580:
case 0x1581:
case 0x1582:
case 0x1583:
case 0x1584:
case 0x1585:
case 0x1586:
case 0x1587:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1588:
case 0x1589:
case 0x158a:
case 0x158b:
case 0x158c:
case 0x158d:
case 0x158e:
case 0x158f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1590:
case 0x1591:
case 0x1592:
case 0x1593:
case 0x1594:
case 0x1595:
case 0x1596:
case 0x1597:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1598:
case 0x1599:
case 0x159a:
case 0x159b:
case 0x159c:
case 0x159d:
case 0x159e:
case 0x159f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x15a0:
case 0x15a1:
case 0x15a2:
case 0x15a3:
case 0x15a4:
case 0x15a5:
case 0x15a6:
case 0x15a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x15a8:
case 0x15a9:
case 0x15aa:
case 0x15ab:
case 0x15ac:
case 0x15ad:
case 0x15ae:
case 0x15af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x15b0:
case 0x15b1:
case 0x15b2:
case 0x15b3:
case 0x15b4:
case 0x15b5:
case 0x15b6:
case 0x15b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x15b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x15b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x15c0:
case 0x15c1:
case 0x15c2:
case 0x15c3:
case 0x15c4:
case 0x15c5:
case 0x15c6:
case 0x15c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15c8:
case 0x15c9:
case 0x15ca:
case 0x15cb:
case 0x15cc:
case 0x15cd:
case 0x15ce:
case 0x15cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15d0:
case 0x15d1:
case 0x15d2:
case 0x15d3:
case 0x15d4:
case 0x15d5:
case 0x15d6:
case 0x15d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15d8:
case 0x15d9:
case 0x15da:
case 0x15db:
case 0x15dc:
case 0x15dd:
case 0x15de:
case 0x15df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15e0:
case 0x15e1:
case 0x15e2:
case 0x15e3:
case 0x15e4:
case 0x15e5:
case 0x15e6:
case 0x15e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15e8:
case 0x15e9:
case 0x15ea:
case 0x15eb:
case 0x15ec:
case 0x15ed:
case 0x15ee:
case 0x15ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15f0:
case 0x15f1:
case 0x15f2:
case 0x15f3:
case 0x15f4:
case 0x15f5:
case 0x15f6:
case 0x15f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x15fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1600:
case 0x1601:
case 0x1602:
case 0x1603:
case 0x1604:
case 0x1605:
case 0x1606:
case 0x1607:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1610:
case 0x1611:
case 0x1612:
case 0x1613:
case 0x1614:
case 0x1615:
case 0x1616:
case 0x1617:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1618:
case 0x1619:
case 0x161a:
case 0x161b:
case 0x161c:
case 0x161d:
case 0x161e:
case 0x161f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1620:
case 0x1621:
case 0x1622:
case 0x1623:
case 0x1624:
case 0x1625:
case 0x1626:
case 0x1627:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1628:
case 0x1629:
case 0x162a:
case 0x162b:
case 0x162c:
case 0x162d:
case 0x162e:
case 0x162f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1630:
case 0x1631:
case 0x1632:
case 0x1633:
case 0x1634:
case 0x1635:
case 0x1636:
case 0x1637:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1638:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1639:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x163a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x163b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x163c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1640:
case 0x1641:
case 0x1642:
case 0x1643:
case 0x1644:
case 0x1645:
case 0x1646:
case 0x1647:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1648:
case 0x1649:
case 0x164a:
case 0x164b:
case 0x164c:
case 0x164d:
case 0x164e:
case 0x164f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1650:
case 0x1651:
case 0x1652:
case 0x1653:
case 0x1654:
case 0x1655:
case 0x1656:
case 0x1657:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1658:
case 0x1659:
case 0x165a:
case 0x165b:
case 0x165c:
case 0x165d:
case 0x165e:
case 0x165f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1660:
case 0x1661:
case 0x1662:
case 0x1663:
case 0x1664:
case 0x1665:
case 0x1666:
case 0x1667:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1668:
case 0x1669:
case 0x166a:
case 0x166b:
case 0x166c:
case 0x166d:
case 0x166e:
case 0x166f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1670:
case 0x1671:
case 0x1672:
case 0x1673:
case 0x1674:
case 0x1675:
case 0x1676:
case 0x1677:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1678:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1679:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x167a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x167b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x167c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1680:
case 0x1681:
case 0x1682:
case 0x1683:
case 0x1684:
case 0x1685:
case 0x1686:
case 0x1687:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1688:
case 0x1689:
case 0x168a:
case 0x168b:
case 0x168c:
case 0x168d:
case 0x168e:
case 0x168f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1690:
case 0x1691:
case 0x1692:
case 0x1693:
case 0x1694:
case 0x1695:
case 0x1696:
case 0x1697:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1698:
case 0x1699:
case 0x169a:
case 0x169b:
case 0x169c:
case 0x169d:
case 0x169e:
case 0x169f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16a0:
case 0x16a1:
case 0x16a2:
case 0x16a3:
case 0x16a4:
case 0x16a5:
case 0x16a6:
case 0x16a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16a8:
case 0x16a9:
case 0x16aa:
case 0x16ab:
case 0x16ac:
case 0x16ad:
case 0x16ae:
case 0x16af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16b0:
case 0x16b1:
case 0x16b2:
case 0x16b3:
case 0x16b4:
case 0x16b5:
case 0x16b6:
case 0x16b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16c0:
case 0x16c1:
case 0x16c2:
case 0x16c3:
case 0x16c4:
case 0x16c5:
case 0x16c6:
case 0x16c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16c8:
case 0x16c9:
case 0x16ca:
case 0x16cb:
case 0x16cc:
case 0x16cd:
case 0x16ce:
case 0x16cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16d0:
case 0x16d1:
case 0x16d2:
case 0x16d3:
case 0x16d4:
case 0x16d5:
case 0x16d6:
case 0x16d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16d8:
case 0x16d9:
case 0x16da:
case 0x16db:
case 0x16dc:
case 0x16dd:
case 0x16de:
case 0x16df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16e0:
case 0x16e1:
case 0x16e2:
case 0x16e3:
case 0x16e4:
case 0x16e5:
case 0x16e6:
case 0x16e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16e8:
case 0x16e9:
case 0x16ea:
case 0x16eb:
case 0x16ec:
case 0x16ed:
case 0x16ee:
case 0x16ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16f0:
case 0x16f1:
case 0x16f2:
case 0x16f3:
case 0x16f4:
case 0x16f5:
case 0x16f6:
case 0x16f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x16fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1700:
case 0x1701:
case 0x1702:
case 0x1703:
case 0x1704:
case 0x1705:
case 0x1706:
case 0x1707:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1708:
case 0x1709:
case 0x170a:
case 0x170b:
case 0x170c:
case 0x170d:
case 0x170e:
case 0x170f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1710:
case 0x1711:
case 0x1712:
case 0x1713:
case 0x1714:
case 0x1715:
case 0x1716:
case 0x1717:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1718:
case 0x1719:
case 0x171a:
case 0x171b:
case 0x171c:
case 0x171d:
case 0x171e:
case 0x171f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1720:
case 0x1721:
case 0x1722:
case 0x1723:
case 0x1724:
case 0x1725:
case 0x1726:
case 0x1727:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1728:
case 0x1729:
case 0x172a:
case 0x172b:
case 0x172c:
case 0x172d:
case 0x172e:
case 0x172f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1730:
case 0x1731:
case 0x1732:
case 0x1733:
case 0x1734:
case 0x1735:
case 0x1736:
case 0x1737:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1738:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1739:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1740:
case 0x1741:
case 0x1742:
case 0x1743:
case 0x1744:
case 0x1745:
case 0x1746:
case 0x1747:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1748:
case 0x1749:
case 0x174a:
case 0x174b:
case 0x174c:
case 0x174d:
case 0x174e:
case 0x174f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1750:
case 0x1751:
case 0x1752:
case 0x1753:
case 0x1754:
case 0x1755:
case 0x1756:
case 0x1757:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1758:
case 0x1759:
case 0x175a:
case 0x175b:
case 0x175c:
case 0x175d:
case 0x175e:
case 0x175f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1760:
case 0x1761:
case 0x1762:
case 0x1763:
case 0x1764:
case 0x1765:
case 0x1766:
case 0x1767:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1768:
case 0x1769:
case 0x176a:
case 0x176b:
case 0x176c:
case 0x176d:
case 0x176e:
case 0x176f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1770:
case 0x1771:
case 0x1772:
case 0x1773:
case 0x1774:
case 0x1775:
case 0x1776:
case 0x1777:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1778:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1779:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1780:
case 0x1781:
case 0x1782:
case 0x1783:
case 0x1784:
case 0x1785:
case 0x1786:
case 0x1787:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1788:
case 0x1789:
case 0x178a:
case 0x178b:
case 0x178c:
case 0x178d:
case 0x178e:
case 0x178f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1790:
case 0x1791:
case 0x1792:
case 0x1793:
case 0x1794:
case 0x1795:
case 0x1796:
case 0x1797:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1798:
case 0x1799:
case 0x179a:
case 0x179b:
case 0x179c:
case 0x179d:
case 0x179e:
case 0x179f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x17a0:
case 0x17a1:
case 0x17a2:
case 0x17a3:
case 0x17a4:
case 0x17a5:
case 0x17a6:
case 0x17a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x17a8:
case 0x17a9:
case 0x17aa:
case 0x17ab:
case 0x17ac:
case 0x17ad:
case 0x17ae:
case 0x17af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x17b0:
case 0x17b1:
case 0x17b2:
case 0x17b3:
case 0x17b4:
case 0x17b5:
case 0x17b6:
case 0x17b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x17b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x17b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x17c0:
case 0x17c1:
case 0x17c2:
case 0x17c3:
case 0x17c4:
case 0x17c5:
case 0x17c6:
case 0x17c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17c8:
case 0x17c9:
case 0x17ca:
case 0x17cb:
case 0x17cc:
case 0x17cd:
case 0x17ce:
case 0x17cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17d0:
case 0x17d1:
case 0x17d2:
case 0x17d3:
case 0x17d4:
case 0x17d5:
case 0x17d6:
case 0x17d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17d8:
case 0x17d9:
case 0x17da:
case 0x17db:
case 0x17dc:
case 0x17dd:
case 0x17de:
case 0x17df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17e0:
case 0x17e1:
case 0x17e2:
case 0x17e3:
case 0x17e4:
case 0x17e5:
case 0x17e6:
case 0x17e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17e8:
case 0x17e9:
case 0x17ea:
case 0x17eb:
case 0x17ec:
case 0x17ed:
case 0x17ee:
case 0x17ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17f0:
case 0x17f1:
case 0x17f2:
case 0x17f3:
case 0x17f4:
case 0x17f5:
case 0x17f6:
case 0x17f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x17fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1800:
case 0x1801:
case 0x1802:
case 0x1803:
case 0x1804:
case 0x1805:
case 0x1806:
case 0x1807:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1810:
case 0x1811:
case 0x1812:
case 0x1813:
case 0x1814:
case 0x1815:
case 0x1816:
case 0x1817:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1818:
case 0x1819:
case 0x181a:
case 0x181b:
case 0x181c:
case 0x181d:
case 0x181e:
case 0x181f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1820:
case 0x1821:
case 0x1822:
case 0x1823:
case 0x1824:
case 0x1825:
case 0x1826:
case 0x1827:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1828:
case 0x1829:
case 0x182a:
case 0x182b:
case 0x182c:
case 0x182d:
case 0x182e:
case 0x182f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1830:
case 0x1831:
case 0x1832:
case 0x1833:
case 0x1834:
case 0x1835:
case 0x1836:
case 0x1837:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1838:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1839:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x183a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x183b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x183c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1840:
case 0x1841:
case 0x1842:
case 0x1843:
case 0x1844:
case 0x1845:
case 0x1846:
case 0x1847:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1848:
case 0x1849:
case 0x184a:
case 0x184b:
case 0x184c:
case 0x184d:
case 0x184e:
case 0x184f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1850:
case 0x1851:
case 0x1852:
case 0x1853:
case 0x1854:
case 0x1855:
case 0x1856:
case 0x1857:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1858:
case 0x1859:
case 0x185a:
case 0x185b:
case 0x185c:
case 0x185d:
case 0x185e:
case 0x185f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1860:
case 0x1861:
case 0x1862:
case 0x1863:
case 0x1864:
case 0x1865:
case 0x1866:
case 0x1867:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1868:
case 0x1869:
case 0x186a:
case 0x186b:
case 0x186c:
case 0x186d:
case 0x186e:
case 0x186f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1870:
case 0x1871:
case 0x1872:
case 0x1873:
case 0x1874:
case 0x1875:
case 0x1876:
case 0x1877:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1878:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1879:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x187a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x187b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x187c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1880:
case 0x1881:
case 0x1882:
case 0x1883:
case 0x1884:
case 0x1885:
case 0x1886:
case 0x1887:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1888:
case 0x1889:
case 0x188a:
case 0x188b:
case 0x188c:
case 0x188d:
case 0x188e:
case 0x188f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1890:
case 0x1891:
case 0x1892:
case 0x1893:
case 0x1894:
case 0x1895:
case 0x1896:
case 0x1897:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1898:
case 0x1899:
case 0x189a:
case 0x189b:
case 0x189c:
case 0x189d:
case 0x189e:
case 0x189f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18a0:
case 0x18a1:
case 0x18a2:
case 0x18a3:
case 0x18a4:
case 0x18a5:
case 0x18a6:
case 0x18a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18a8:
case 0x18a9:
case 0x18aa:
case 0x18ab:
case 0x18ac:
case 0x18ad:
case 0x18ae:
case 0x18af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18b0:
case 0x18b1:
case 0x18b2:
case 0x18b3:
case 0x18b4:
case 0x18b5:
case 0x18b6:
case 0x18b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18c0:
case 0x18c1:
case 0x18c2:
case 0x18c3:
case 0x18c4:
case 0x18c5:
case 0x18c6:
case 0x18c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18c8:
case 0x18c9:
case 0x18ca:
case 0x18cb:
case 0x18cc:
case 0x18cd:
case 0x18ce:
case 0x18cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18d0:
case 0x18d1:
case 0x18d2:
case 0x18d3:
case 0x18d4:
case 0x18d5:
case 0x18d6:
case 0x18d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18d8:
case 0x18d9:
case 0x18da:
case 0x18db:
case 0x18dc:
case 0x18dd:
case 0x18de:
case 0x18df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18e0:
case 0x18e1:
case 0x18e2:
case 0x18e3:
case 0x18e4:
case 0x18e5:
case 0x18e6:
case 0x18e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18e8:
case 0x18e9:
case 0x18ea:
case 0x18eb:
case 0x18ec:
case 0x18ed:
case 0x18ee:
case 0x18ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18f0:
case 0x18f1:
case 0x18f2:
case 0x18f3:
case 0x18f4:
case 0x18f5:
case 0x18f6:
case 0x18f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x18fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1900:
case 0x1901:
case 0x1902:
case 0x1903:
case 0x1904:
case 0x1905:
case 0x1906:
case 0x1907:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1908:
case 0x1909:
case 0x190a:
case 0x190b:
case 0x190c:
case 0x190d:
case 0x190e:
case 0x190f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1910:
case 0x1911:
case 0x1912:
case 0x1913:
case 0x1914:
case 0x1915:
case 0x1916:
case 0x1917:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1918:
case 0x1919:
case 0x191a:
case 0x191b:
case 0x191c:
case 0x191d:
case 0x191e:
case 0x191f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1920:
case 0x1921:
case 0x1922:
case 0x1923:
case 0x1924:
case 0x1925:
case 0x1926:
case 0x1927:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1928:
case 0x1929:
case 0x192a:
case 0x192b:
case 0x192c:
case 0x192d:
case 0x192e:
case 0x192f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1930:
case 0x1931:
case 0x1932:
case 0x1933:
case 0x1934:
case 0x1935:
case 0x1936:
case 0x1937:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1938:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1939:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1940:
case 0x1941:
case 0x1942:
case 0x1943:
case 0x1944:
case 0x1945:
case 0x1946:
case 0x1947:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1948:
case 0x1949:
case 0x194a:
case 0x194b:
case 0x194c:
case 0x194d:
case 0x194e:
case 0x194f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1950:
case 0x1951:
case 0x1952:
case 0x1953:
case 0x1954:
case 0x1955:
case 0x1956:
case 0x1957:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1958:
case 0x1959:
case 0x195a:
case 0x195b:
case 0x195c:
case 0x195d:
case 0x195e:
case 0x195f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1960:
case 0x1961:
case 0x1962:
case 0x1963:
case 0x1964:
case 0x1965:
case 0x1966:
case 0x1967:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1968:
case 0x1969:
case 0x196a:
case 0x196b:
case 0x196c:
case 0x196d:
case 0x196e:
case 0x196f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1970:
case 0x1971:
case 0x1972:
case 0x1973:
case 0x1974:
case 0x1975:
case 0x1976:
case 0x1977:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1978:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1979:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1980:
case 0x1981:
case 0x1982:
case 0x1983:
case 0x1984:
case 0x1985:
case 0x1986:
case 0x1987:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1988:
case 0x1989:
case 0x198a:
case 0x198b:
case 0x198c:
case 0x198d:
case 0x198e:
case 0x198f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1990:
case 0x1991:
case 0x1992:
case 0x1993:
case 0x1994:
case 0x1995:
case 0x1996:
case 0x1997:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1998:
case 0x1999:
case 0x199a:
case 0x199b:
case 0x199c:
case 0x199d:
case 0x199e:
case 0x199f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x19a0:
case 0x19a1:
case 0x19a2:
case 0x19a3:
case 0x19a4:
case 0x19a5:
case 0x19a6:
case 0x19a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x19a8:
case 0x19a9:
case 0x19aa:
case 0x19ab:
case 0x19ac:
case 0x19ad:
case 0x19ae:
case 0x19af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x19b0:
case 0x19b1:
case 0x19b2:
case 0x19b3:
case 0x19b4:
case 0x19b5:
case 0x19b6:
case 0x19b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x19b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x19b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x19c0:
case 0x19c1:
case 0x19c2:
case 0x19c3:
case 0x19c4:
case 0x19c5:
case 0x19c6:
case 0x19c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19c8:
case 0x19c9:
case 0x19ca:
case 0x19cb:
case 0x19cc:
case 0x19cd:
case 0x19ce:
case 0x19cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19d0:
case 0x19d1:
case 0x19d2:
case 0x19d3:
case 0x19d4:
case 0x19d5:
case 0x19d6:
case 0x19d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19d8:
case 0x19d9:
case 0x19da:
case 0x19db:
case 0x19dc:
case 0x19dd:
case 0x19de:
case 0x19df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19e0:
case 0x19e1:
case 0x19e2:
case 0x19e3:
case 0x19e4:
case 0x19e5:
case 0x19e6:
case 0x19e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19e8:
case 0x19e9:
case 0x19ea:
case 0x19eb:
case 0x19ec:
case 0x19ed:
case 0x19ee:
case 0x19ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19f0:
case 0x19f1:
case 0x19f2:
case 0x19f3:
case 0x19f4:
case 0x19f5:
case 0x19f6:
case 0x19f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x19fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a00:
case 0x1a01:
case 0x1a02:
case 0x1a03:
case 0x1a04:
case 0x1a05:
case 0x1a06:
case 0x1a07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a10:
case 0x1a11:
case 0x1a12:
case 0x1a13:
case 0x1a14:
case 0x1a15:
case 0x1a16:
case 0x1a17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a18:
case 0x1a19:
case 0x1a1a:
case 0x1a1b:
case 0x1a1c:
case 0x1a1d:
case 0x1a1e:
case 0x1a1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a20:
case 0x1a21:
case 0x1a22:
case 0x1a23:
case 0x1a24:
case 0x1a25:
case 0x1a26:
case 0x1a27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a28:
case 0x1a29:
case 0x1a2a:
case 0x1a2b:
case 0x1a2c:
case 0x1a2d:
case 0x1a2e:
case 0x1a2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a30:
case 0x1a31:
case 0x1a32:
case 0x1a33:
case 0x1a34:
case 0x1a35:
case 0x1a36:
case 0x1a37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a40:
case 0x1a41:
case 0x1a42:
case 0x1a43:
case 0x1a44:
case 0x1a45:
case 0x1a46:
case 0x1a47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a48:
case 0x1a49:
case 0x1a4a:
case 0x1a4b:
case 0x1a4c:
case 0x1a4d:
case 0x1a4e:
case 0x1a4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a50:
case 0x1a51:
case 0x1a52:
case 0x1a53:
case 0x1a54:
case 0x1a55:
case 0x1a56:
case 0x1a57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a58:
case 0x1a59:
case 0x1a5a:
case 0x1a5b:
case 0x1a5c:
case 0x1a5d:
case 0x1a5e:
case 0x1a5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a60:
case 0x1a61:
case 0x1a62:
case 0x1a63:
case 0x1a64:
case 0x1a65:
case 0x1a66:
case 0x1a67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a68:
case 0x1a69:
case 0x1a6a:
case 0x1a6b:
case 0x1a6c:
case 0x1a6d:
case 0x1a6e:
case 0x1a6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a70:
case 0x1a71:
case 0x1a72:
case 0x1a73:
case 0x1a74:
case 0x1a75:
case 0x1a76:
case 0x1a77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a80:
case 0x1a81:
case 0x1a82:
case 0x1a83:
case 0x1a84:
case 0x1a85:
case 0x1a86:
case 0x1a87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a88:
case 0x1a89:
case 0x1a8a:
case 0x1a8b:
case 0x1a8c:
case 0x1a8d:
case 0x1a8e:
case 0x1a8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a90:
case 0x1a91:
case 0x1a92:
case 0x1a93:
case 0x1a94:
case 0x1a95:
case 0x1a96:
case 0x1a97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1a98:
case 0x1a99:
case 0x1a9a:
case 0x1a9b:
case 0x1a9c:
case 0x1a9d:
case 0x1a9e:
case 0x1a9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1aa0:
case 0x1aa1:
case 0x1aa2:
case 0x1aa3:
case 0x1aa4:
case 0x1aa5:
case 0x1aa6:
case 0x1aa7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1aa8:
case 0x1aa9:
case 0x1aaa:
case 0x1aab:
case 0x1aac:
case 0x1aad:
case 0x1aae:
case 0x1aaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ab0:
case 0x1ab1:
case 0x1ab2:
case 0x1ab3:
case 0x1ab4:
case 0x1ab5:
case 0x1ab6:
case 0x1ab7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ab8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ab9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1aba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1abb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1abc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ac0:
case 0x1ac1:
case 0x1ac2:
case 0x1ac3:
case 0x1ac4:
case 0x1ac5:
case 0x1ac6:
case 0x1ac7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ac8:
case 0x1ac9:
case 0x1aca:
case 0x1acb:
case 0x1acc:
case 0x1acd:
case 0x1ace:
case 0x1acf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ad0:
case 0x1ad1:
case 0x1ad2:
case 0x1ad3:
case 0x1ad4:
case 0x1ad5:
case 0x1ad6:
case 0x1ad7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ad8:
case 0x1ad9:
case 0x1ada:
case 0x1adb:
case 0x1adc:
case 0x1add:
case 0x1ade:
case 0x1adf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ae0:
case 0x1ae1:
case 0x1ae2:
case 0x1ae3:
case 0x1ae4:
case 0x1ae5:
case 0x1ae6:
case 0x1ae7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ae8:
case 0x1ae9:
case 0x1aea:
case 0x1aeb:
case 0x1aec:
case 0x1aed:
case 0x1aee:
case 0x1aef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1af0:
case 0x1af1:
case 0x1af2:
case 0x1af3:
case 0x1af4:
case 0x1af5:
case 0x1af6:
case 0x1af7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1af8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1af9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1afa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1afb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1afc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1b00:
case 0x1b01:
case 0x1b02:
case 0x1b03:
case 0x1b04:
case 0x1b05:
case 0x1b06:
case 0x1b07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b08:
case 0x1b09:
case 0x1b0a:
case 0x1b0b:
case 0x1b0c:
case 0x1b0d:
case 0x1b0e:
case 0x1b0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b10:
case 0x1b11:
case 0x1b12:
case 0x1b13:
case 0x1b14:
case 0x1b15:
case 0x1b16:
case 0x1b17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b18:
case 0x1b19:
case 0x1b1a:
case 0x1b1b:
case 0x1b1c:
case 0x1b1d:
case 0x1b1e:
case 0x1b1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b20:
case 0x1b21:
case 0x1b22:
case 0x1b23:
case 0x1b24:
case 0x1b25:
case 0x1b26:
case 0x1b27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b28:
case 0x1b29:
case 0x1b2a:
case 0x1b2b:
case 0x1b2c:
case 0x1b2d:
case 0x1b2e:
case 0x1b2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b30:
case 0x1b31:
case 0x1b32:
case 0x1b33:
case 0x1b34:
case 0x1b35:
case 0x1b36:
case 0x1b37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1b39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1b40:
case 0x1b41:
case 0x1b42:
case 0x1b43:
case 0x1b44:
case 0x1b45:
case 0x1b46:
case 0x1b47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b48:
case 0x1b49:
case 0x1b4a:
case 0x1b4b:
case 0x1b4c:
case 0x1b4d:
case 0x1b4e:
case 0x1b4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b50:
case 0x1b51:
case 0x1b52:
case 0x1b53:
case 0x1b54:
case 0x1b55:
case 0x1b56:
case 0x1b57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b58:
case 0x1b59:
case 0x1b5a:
case 0x1b5b:
case 0x1b5c:
case 0x1b5d:
case 0x1b5e:
case 0x1b5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b60:
case 0x1b61:
case 0x1b62:
case 0x1b63:
case 0x1b64:
case 0x1b65:
case 0x1b66:
case 0x1b67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b68:
case 0x1b69:
case 0x1b6a:
case 0x1b6b:
case 0x1b6c:
case 0x1b6d:
case 0x1b6e:
case 0x1b6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b70:
case 0x1b71:
case 0x1b72:
case 0x1b73:
case 0x1b74:
case 0x1b75:
case 0x1b76:
case 0x1b77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1b79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1b80:
case 0x1b81:
case 0x1b82:
case 0x1b83:
case 0x1b84:
case 0x1b85:
case 0x1b86:
case 0x1b87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b88:
case 0x1b89:
case 0x1b8a:
case 0x1b8b:
case 0x1b8c:
case 0x1b8d:
case 0x1b8e:
case 0x1b8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1b90:
case 0x1b91:
case 0x1b92:
case 0x1b93:
case 0x1b94:
case 0x1b95:
case 0x1b96:
case 0x1b97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1b98:
case 0x1b99:
case 0x1b9a:
case 0x1b9b:
case 0x1b9c:
case 0x1b9d:
case 0x1b9e:
case 0x1b9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1ba0:
case 0x1ba1:
case 0x1ba2:
case 0x1ba3:
case 0x1ba4:
case 0x1ba5:
case 0x1ba6:
case 0x1ba7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1ba8:
case 0x1ba9:
case 0x1baa:
case 0x1bab:
case 0x1bac:
case 0x1bad:
case 0x1bae:
case 0x1baf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1bb0:
case 0x1bb1:
case 0x1bb2:
case 0x1bb3:
case 0x1bb4:
case 0x1bb5:
case 0x1bb6:
case 0x1bb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1bb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1bb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1bc0:
case 0x1bc1:
case 0x1bc2:
case 0x1bc3:
case 0x1bc4:
case 0x1bc5:
case 0x1bc6:
case 0x1bc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bc8:
case 0x1bc9:
case 0x1bca:
case 0x1bcb:
case 0x1bcc:
case 0x1bcd:
case 0x1bce:
case 0x1bcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bd0:
case 0x1bd1:
case 0x1bd2:
case 0x1bd3:
case 0x1bd4:
case 0x1bd5:
case 0x1bd6:
case 0x1bd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bd8:
case 0x1bd9:
case 0x1bda:
case 0x1bdb:
case 0x1bdc:
case 0x1bdd:
case 0x1bde:
case 0x1bdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1be0:
case 0x1be1:
case 0x1be2:
case 0x1be3:
case 0x1be4:
case 0x1be5:
case 0x1be6:
case 0x1be7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1be8:
case 0x1be9:
case 0x1bea:
case 0x1beb:
case 0x1bec:
case 0x1bed:
case 0x1bee:
case 0x1bef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bf0:
case 0x1bf1:
case 0x1bf2:
case 0x1bf3:
case 0x1bf4:
case 0x1bf5:
case 0x1bf6:
case 0x1bf7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bf8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bf9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1bfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c00:
case 0x1c01:
case 0x1c02:
case 0x1c03:
case 0x1c04:
case 0x1c05:
case 0x1c06:
case 0x1c07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c10:
case 0x1c11:
case 0x1c12:
case 0x1c13:
case 0x1c14:
case 0x1c15:
case 0x1c16:
case 0x1c17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c18:
case 0x1c19:
case 0x1c1a:
case 0x1c1b:
case 0x1c1c:
case 0x1c1d:
case 0x1c1e:
case 0x1c1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c20:
case 0x1c21:
case 0x1c22:
case 0x1c23:
case 0x1c24:
case 0x1c25:
case 0x1c26:
case 0x1c27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c28:
case 0x1c29:
case 0x1c2a:
case 0x1c2b:
case 0x1c2c:
case 0x1c2d:
case 0x1c2e:
case 0x1c2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c30:
case 0x1c31:
case 0x1c32:
case 0x1c33:
case 0x1c34:
case 0x1c35:
case 0x1c36:
case 0x1c37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c40:
case 0x1c41:
case 0x1c42:
case 0x1c43:
case 0x1c44:
case 0x1c45:
case 0x1c46:
case 0x1c47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c48:
case 0x1c49:
case 0x1c4a:
case 0x1c4b:
case 0x1c4c:
case 0x1c4d:
case 0x1c4e:
case 0x1c4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c50:
case 0x1c51:
case 0x1c52:
case 0x1c53:
case 0x1c54:
case 0x1c55:
case 0x1c56:
case 0x1c57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c58:
case 0x1c59:
case 0x1c5a:
case 0x1c5b:
case 0x1c5c:
case 0x1c5d:
case 0x1c5e:
case 0x1c5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c60:
case 0x1c61:
case 0x1c62:
case 0x1c63:
case 0x1c64:
case 0x1c65:
case 0x1c66:
case 0x1c67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c68:
case 0x1c69:
case 0x1c6a:
case 0x1c6b:
case 0x1c6c:
case 0x1c6d:
case 0x1c6e:
case 0x1c6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c70:
case 0x1c71:
case 0x1c72:
case 0x1c73:
case 0x1c74:
case 0x1c75:
case 0x1c76:
case 0x1c77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c80:
case 0x1c81:
case 0x1c82:
case 0x1c83:
case 0x1c84:
case 0x1c85:
case 0x1c86:
case 0x1c87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c88:
case 0x1c89:
case 0x1c8a:
case 0x1c8b:
case 0x1c8c:
case 0x1c8d:
case 0x1c8e:
case 0x1c8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c90:
case 0x1c91:
case 0x1c92:
case 0x1c93:
case 0x1c94:
case 0x1c95:
case 0x1c96:
case 0x1c97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1c98:
case 0x1c99:
case 0x1c9a:
case 0x1c9b:
case 0x1c9c:
case 0x1c9d:
case 0x1c9e:
case 0x1c9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ca0:
case 0x1ca1:
case 0x1ca2:
case 0x1ca3:
case 0x1ca4:
case 0x1ca5:
case 0x1ca6:
case 0x1ca7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ca8:
case 0x1ca9:
case 0x1caa:
case 0x1cab:
case 0x1cac:
case 0x1cad:
case 0x1cae:
case 0x1caf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cb0:
case 0x1cb1:
case 0x1cb2:
case 0x1cb3:
case 0x1cb4:
case 0x1cb5:
case 0x1cb6:
case 0x1cb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cbb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cbc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cc0:
case 0x1cc1:
case 0x1cc2:
case 0x1cc3:
case 0x1cc4:
case 0x1cc5:
case 0x1cc6:
case 0x1cc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cc8:
case 0x1cc9:
case 0x1cca:
case 0x1ccb:
case 0x1ccc:
case 0x1ccd:
case 0x1cce:
case 0x1ccf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cd0:
case 0x1cd1:
case 0x1cd2:
case 0x1cd3:
case 0x1cd4:
case 0x1cd5:
case 0x1cd6:
case 0x1cd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cd8:
case 0x1cd9:
case 0x1cda:
case 0x1cdb:
case 0x1cdc:
case 0x1cdd:
case 0x1cde:
case 0x1cdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ce0:
case 0x1ce1:
case 0x1ce2:
case 0x1ce3:
case 0x1ce4:
case 0x1ce5:
case 0x1ce6:
case 0x1ce7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ce8:
case 0x1ce9:
case 0x1cea:
case 0x1ceb:
case 0x1cec:
case 0x1ced:
case 0x1cee:
case 0x1cef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cf0:
case 0x1cf1:
case 0x1cf2:
case 0x1cf3:
case 0x1cf4:
case 0x1cf5:
case 0x1cf6:
case 0x1cf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cf9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cfa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cfb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1cfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1d00:
case 0x1d01:
case 0x1d02:
case 0x1d03:
case 0x1d04:
case 0x1d05:
case 0x1d06:
case 0x1d07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d08:
case 0x1d09:
case 0x1d0a:
case 0x1d0b:
case 0x1d0c:
case 0x1d0d:
case 0x1d0e:
case 0x1d0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d10:
case 0x1d11:
case 0x1d12:
case 0x1d13:
case 0x1d14:
case 0x1d15:
case 0x1d16:
case 0x1d17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d18:
case 0x1d19:
case 0x1d1a:
case 0x1d1b:
case 0x1d1c:
case 0x1d1d:
case 0x1d1e:
case 0x1d1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d20:
case 0x1d21:
case 0x1d22:
case 0x1d23:
case 0x1d24:
case 0x1d25:
case 0x1d26:
case 0x1d27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d28:
case 0x1d29:
case 0x1d2a:
case 0x1d2b:
case 0x1d2c:
case 0x1d2d:
case 0x1d2e:
case 0x1d2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d30:
case 0x1d31:
case 0x1d32:
case 0x1d33:
case 0x1d34:
case 0x1d35:
case 0x1d36:
case 0x1d37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1d39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1d40:
case 0x1d41:
case 0x1d42:
case 0x1d43:
case 0x1d44:
case 0x1d45:
case 0x1d46:
case 0x1d47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d48:
case 0x1d49:
case 0x1d4a:
case 0x1d4b:
case 0x1d4c:
case 0x1d4d:
case 0x1d4e:
case 0x1d4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d50:
case 0x1d51:
case 0x1d52:
case 0x1d53:
case 0x1d54:
case 0x1d55:
case 0x1d56:
case 0x1d57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d58:
case 0x1d59:
case 0x1d5a:
case 0x1d5b:
case 0x1d5c:
case 0x1d5d:
case 0x1d5e:
case 0x1d5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d60:
case 0x1d61:
case 0x1d62:
case 0x1d63:
case 0x1d64:
case 0x1d65:
case 0x1d66:
case 0x1d67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d68:
case 0x1d69:
case 0x1d6a:
case 0x1d6b:
case 0x1d6c:
case 0x1d6d:
case 0x1d6e:
case 0x1d6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d70:
case 0x1d71:
case 0x1d72:
case 0x1d73:
case 0x1d74:
case 0x1d75:
case 0x1d76:
case 0x1d77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1d79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1d80:
case 0x1d81:
case 0x1d82:
case 0x1d83:
case 0x1d84:
case 0x1d85:
case 0x1d86:
case 0x1d87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d88:
case 0x1d89:
case 0x1d8a:
case 0x1d8b:
case 0x1d8c:
case 0x1d8d:
case 0x1d8e:
case 0x1d8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1d90:
case 0x1d91:
case 0x1d92:
case 0x1d93:
case 0x1d94:
case 0x1d95:
case 0x1d96:
case 0x1d97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1d98:
case 0x1d99:
case 0x1d9a:
case 0x1d9b:
case 0x1d9c:
case 0x1d9d:
case 0x1d9e:
case 0x1d9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1da0:
case 0x1da1:
case 0x1da2:
case 0x1da3:
case 0x1da4:
case 0x1da5:
case 0x1da6:
case 0x1da7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1da8:
case 0x1da9:
case 0x1daa:
case 0x1dab:
case 0x1dac:
case 0x1dad:
case 0x1dae:
case 0x1daf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1db0:
case 0x1db1:
case 0x1db2:
case 0x1db3:
case 0x1db4:
case 0x1db5:
case 0x1db6:
case 0x1db7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1db8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1db9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1dc0:
case 0x1dc1:
case 0x1dc2:
case 0x1dc3:
case 0x1dc4:
case 0x1dc5:
case 0x1dc6:
case 0x1dc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dc8:
case 0x1dc9:
case 0x1dca:
case 0x1dcb:
case 0x1dcc:
case 0x1dcd:
case 0x1dce:
case 0x1dcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dd0:
case 0x1dd1:
case 0x1dd2:
case 0x1dd3:
case 0x1dd4:
case 0x1dd5:
case 0x1dd6:
case 0x1dd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dd8:
case 0x1dd9:
case 0x1dda:
case 0x1ddb:
case 0x1ddc:
case 0x1ddd:
case 0x1dde:
case 0x1ddf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1de0:
case 0x1de1:
case 0x1de2:
case 0x1de3:
case 0x1de4:
case 0x1de5:
case 0x1de6:
case 0x1de7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1de8:
case 0x1de9:
case 0x1dea:
case 0x1deb:
case 0x1dec:
case 0x1ded:
case 0x1dee:
case 0x1def:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1df0:
case 0x1df1:
case 0x1df2:
case 0x1df3:
case 0x1df4:
case 0x1df5:
case 0x1df6:
case 0x1df7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1df8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1df9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1dfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e00:
case 0x1e01:
case 0x1e02:
case 0x1e03:
case 0x1e04:
case 0x1e05:
case 0x1e06:
case 0x1e07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e10:
case 0x1e11:
case 0x1e12:
case 0x1e13:
case 0x1e14:
case 0x1e15:
case 0x1e16:
case 0x1e17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e18:
case 0x1e19:
case 0x1e1a:
case 0x1e1b:
case 0x1e1c:
case 0x1e1d:
case 0x1e1e:
case 0x1e1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e20:
case 0x1e21:
case 0x1e22:
case 0x1e23:
case 0x1e24:
case 0x1e25:
case 0x1e26:
case 0x1e27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e28:
case 0x1e29:
case 0x1e2a:
case 0x1e2b:
case 0x1e2c:
case 0x1e2d:
case 0x1e2e:
case 0x1e2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e30:
case 0x1e31:
case 0x1e32:
case 0x1e33:
case 0x1e34:
case 0x1e35:
case 0x1e36:
case 0x1e37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e40:
case 0x1e41:
case 0x1e42:
case 0x1e43:
case 0x1e44:
case 0x1e45:
case 0x1e46:
case 0x1e47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e48:
case 0x1e49:
case 0x1e4a:
case 0x1e4b:
case 0x1e4c:
case 0x1e4d:
case 0x1e4e:
case 0x1e4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e50:
case 0x1e51:
case 0x1e52:
case 0x1e53:
case 0x1e54:
case 0x1e55:
case 0x1e56:
case 0x1e57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e58:
case 0x1e59:
case 0x1e5a:
case 0x1e5b:
case 0x1e5c:
case 0x1e5d:
case 0x1e5e:
case 0x1e5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e60:
case 0x1e61:
case 0x1e62:
case 0x1e63:
case 0x1e64:
case 0x1e65:
case 0x1e66:
case 0x1e67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e68:
case 0x1e69:
case 0x1e6a:
case 0x1e6b:
case 0x1e6c:
case 0x1e6d:
case 0x1e6e:
case 0x1e6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e70:
case 0x1e71:
case 0x1e72:
case 0x1e73:
case 0x1e74:
case 0x1e75:
case 0x1e76:
case 0x1e77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e80:
case 0x1e81:
case 0x1e82:
case 0x1e83:
case 0x1e84:
case 0x1e85:
case 0x1e86:
case 0x1e87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e88:
case 0x1e89:
case 0x1e8a:
case 0x1e8b:
case 0x1e8c:
case 0x1e8d:
case 0x1e8e:
case 0x1e8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e90:
case 0x1e91:
case 0x1e92:
case 0x1e93:
case 0x1e94:
case 0x1e95:
case 0x1e96:
case 0x1e97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1e98:
case 0x1e99:
case 0x1e9a:
case 0x1e9b:
case 0x1e9c:
case 0x1e9d:
case 0x1e9e:
case 0x1e9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ea0:
case 0x1ea1:
case 0x1ea2:
case 0x1ea3:
case 0x1ea4:
case 0x1ea5:
case 0x1ea6:
case 0x1ea7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ea8:
case 0x1ea9:
case 0x1eaa:
case 0x1eab:
case 0x1eac:
case 0x1ead:
case 0x1eae:
case 0x1eaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1eb0:
case 0x1eb1:
case 0x1eb2:
case 0x1eb3:
case 0x1eb4:
case 0x1eb5:
case 0x1eb6:
case 0x1eb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1eb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1eb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1eba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ebb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ebc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ec0:
case 0x1ec1:
case 0x1ec2:
case 0x1ec3:
case 0x1ec4:
case 0x1ec5:
case 0x1ec6:
case 0x1ec7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ec8:
case 0x1ec9:
case 0x1eca:
case 0x1ecb:
case 0x1ecc:
case 0x1ecd:
case 0x1ece:
case 0x1ecf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ed0:
case 0x1ed1:
case 0x1ed2:
case 0x1ed3:
case 0x1ed4:
case 0x1ed5:
case 0x1ed6:
case 0x1ed7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ed8:
case 0x1ed9:
case 0x1eda:
case 0x1edb:
case 0x1edc:
case 0x1edd:
case 0x1ede:
case 0x1edf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ee0:
case 0x1ee1:
case 0x1ee2:
case 0x1ee3:
case 0x1ee4:
case 0x1ee5:
case 0x1ee6:
case 0x1ee7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ee8:
case 0x1ee9:
case 0x1eea:
case 0x1eeb:
case 0x1eec:
case 0x1eed:
case 0x1eee:
case 0x1eef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ef0:
case 0x1ef1:
case 0x1ef2:
case 0x1ef3:
case 0x1ef4:
case 0x1ef5:
case 0x1ef6:
case 0x1ef7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ef8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ef9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1efa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1efb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1efc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1f00:
case 0x1f01:
case 0x1f02:
case 0x1f03:
case 0x1f04:
case 0x1f05:
case 0x1f06:
case 0x1f07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f08:
case 0x1f09:
case 0x1f0a:
case 0x1f0b:
case 0x1f0c:
case 0x1f0d:
case 0x1f0e:
case 0x1f0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f10:
case 0x1f11:
case 0x1f12:
case 0x1f13:
case 0x1f14:
case 0x1f15:
case 0x1f16:
case 0x1f17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f18:
case 0x1f19:
case 0x1f1a:
case 0x1f1b:
case 0x1f1c:
case 0x1f1d:
case 0x1f1e:
case 0x1f1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f20:
case 0x1f21:
case 0x1f22:
case 0x1f23:
case 0x1f24:
case 0x1f25:
case 0x1f26:
case 0x1f27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f28:
case 0x1f29:
case 0x1f2a:
case 0x1f2b:
case 0x1f2c:
case 0x1f2d:
case 0x1f2e:
case 0x1f2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f30:
case 0x1f31:
case 0x1f32:
case 0x1f33:
case 0x1f34:
case 0x1f35:
case 0x1f36:
case 0x1f37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1f39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1f40:
case 0x1f41:
case 0x1f42:
case 0x1f43:
case 0x1f44:
case 0x1f45:
case 0x1f46:
case 0x1f47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f48:
case 0x1f49:
case 0x1f4a:
case 0x1f4b:
case 0x1f4c:
case 0x1f4d:
case 0x1f4e:
case 0x1f4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f50:
case 0x1f51:
case 0x1f52:
case 0x1f53:
case 0x1f54:
case 0x1f55:
case 0x1f56:
case 0x1f57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f58:
case 0x1f59:
case 0x1f5a:
case 0x1f5b:
case 0x1f5c:
case 0x1f5d:
case 0x1f5e:
case 0x1f5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f60:
case 0x1f61:
case 0x1f62:
case 0x1f63:
case 0x1f64:
case 0x1f65:
case 0x1f66:
case 0x1f67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f68:
case 0x1f69:
case 0x1f6a:
case 0x1f6b:
case 0x1f6c:
case 0x1f6d:
case 0x1f6e:
case 0x1f6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f70:
case 0x1f71:
case 0x1f72:
case 0x1f73:
case 0x1f74:
case 0x1f75:
case 0x1f76:
case 0x1f77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1f79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1f80:
case 0x1f81:
case 0x1f82:
case 0x1f83:
case 0x1f84:
case 0x1f85:
case 0x1f86:
case 0x1f87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f88:
case 0x1f89:
case 0x1f8a:
case 0x1f8b:
case 0x1f8c:
case 0x1f8d:
case 0x1f8e:
case 0x1f8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); SUBX(src, dst);
	}
	break;

case 0x1f90:
case 0x1f91:
case 0x1f92:
case 0x1f93:
case 0x1f94:
case 0x1f95:
case 0x1f96:
case 0x1f97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1f98:
case 0x1f99:
case 0x1f9a:
case 0x1f9b:
case 0x1f9c:
case 0x1f9d:
case 0x1f9e:
case 0x1f9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1fa0:
case 0x1fa1:
case 0x1fa2:
case 0x1fa3:
case 0x1fa4:
case 0x1fa5:
case 0x1fa6:
case 0x1fa7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1fa8:
case 0x1fa9:
case 0x1faa:
case 0x1fab:
case 0x1fac:
case 0x1fad:
case 0x1fae:
case 0x1faf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1fb0:
case 0x1fb1:
case 0x1fb2:
case 0x1fb3:
case 0x1fb4:
case 0x1fb5:
case 0x1fb6:
case 0x1fb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); SUB(src, dst);
	}
	break;

case 0x1fb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); SUB(src, dst);
	}
	break;

case 0x1fb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); SUB(src, dst);
	}
	break;

case 0x1fc0:
case 0x1fc1:
case 0x1fc2:
case 0x1fc3:
case 0x1fc4:
case 0x1fc5:
case 0x1fc6:
case 0x1fc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1fc8:
case 0x1fc9:
case 0x1fca:
case 0x1fcb:
case 0x1fcc:
case 0x1fcd:
case 0x1fce:
case 0x1fcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1fd0:
case 0x1fd1:
case 0x1fd2:
case 0x1fd3:
case 0x1fd4:
case 0x1fd5:
case 0x1fd6:
case 0x1fd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1fd8:
case 0x1fd9:
case 0x1fda:
case 0x1fdb:
case 0x1fdc:
case 0x1fdd:
case 0x1fde:
case 0x1fdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1fe0:
case 0x1fe1:
case 0x1fe2:
case 0x1fe3:
case 0x1fe4:
case 0x1fe5:
case 0x1fe6:
case 0x1fe7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1fe8:
case 0x1fe9:
case 0x1fea:
case 0x1feb:
case 0x1fec:
case 0x1fed:
case 0x1fee:
case 0x1fef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ff0:
case 0x1ff1:
case 0x1ff2:
case 0x1ff3:
case 0x1ff4:
case 0x1ff5:
case 0x1ff6:
case 0x1ff7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ff8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ff9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ffa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ffb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x1ffc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); SUB(src, dst);
	}
	break;

case 0x2000:
case 0x2001:
case 0x2002:
case 0x2003:
case 0x2004:
case 0x2005:
case 0x2006:
case 0x2007:
case 0x2008:
case 0x2009:
case 0x200a:
case 0x200b:
case 0x200c:
case 0x200d:
case 0x200e:
case 0x200f:
case 0x2010:
case 0x2011:
case 0x2012:
case 0x2013:
case 0x2014:
case 0x2015:
case 0x2016:
case 0x2017:
case 0x2018:
case 0x2019:
case 0x201a:
case 0x201b:
case 0x201c:
case 0x201d:
case 0x201e:
case 0x201f:
case 0x2020:
case 0x2021:
case 0x2022:
case 0x2023:
case 0x2024:
case 0x2025:
case 0x2026:
case 0x2027:
case 0x2028:
case 0x2029:
case 0x202a:
case 0x202b:
case 0x202c:
case 0x202d:
case 0x202e:
case 0x202f:
case 0x2030:
case 0x2031:
case 0x2032:
case 0x2033:
case 0x2034:
case 0x2035:
case 0x2036:
case 0x2037:
case 0x2038:
case 0x2039:
case 0x203a:
case 0x203b:
case 0x203c:
case 0x203d:
case 0x203e:
case 0x203f:
case 0x2040:
case 0x2041:
case 0x2042:
case 0x2043:
case 0x2044:
case 0x2045:
case 0x2046:
case 0x2047:
case 0x2048:
case 0x2049:
case 0x204a:
case 0x204b:
case 0x204c:
case 0x204d:
case 0x204e:
case 0x204f:
case 0x2050:
case 0x2051:
case 0x2052:
case 0x2053:
case 0x2054:
case 0x2055:
case 0x2056:
case 0x2057:
case 0x2058:
case 0x2059:
case 0x205a:
case 0x205b:
case 0x205c:
case 0x205d:
case 0x205e:
case 0x205f:
case 0x2060:
case 0x2061:
case 0x2062:
case 0x2063:
case 0x2064:
case 0x2065:
case 0x2066:
case 0x2067:
case 0x2068:
case 0x2069:
case 0x206a:
case 0x206b:
case 0x206c:
case 0x206d:
case 0x206e:
case 0x206f:
case 0x2070:
case 0x2071:
case 0x2072:
case 0x2073:
case 0x2074:
case 0x2075:
case 0x2076:
case 0x2077:
case 0x2078:
case 0x2079:
case 0x207a:
case 0x207b:
case 0x207c:
case 0x207d:
case 0x207e:
case 0x207f:
case 0x2080:
case 0x2081:
case 0x2082:
case 0x2083:
case 0x2084:
case 0x2085:
case 0x2086:
case 0x2087:
case 0x2088:
case 0x2089:
case 0x208a:
case 0x208b:
case 0x208c:
case 0x208d:
case 0x208e:
case 0x208f:
case 0x2090:
case 0x2091:
case 0x2092:
case 0x2093:
case 0x2094:
case 0x2095:
case 0x2096:
case 0x2097:
case 0x2098:
case 0x2099:
case 0x209a:
case 0x209b:
case 0x209c:
case 0x209d:
case 0x209e:
case 0x209f:
case 0x20a0:
case 0x20a1:
case 0x20a2:
case 0x20a3:
case 0x20a4:
case 0x20a5:
case 0x20a6:
case 0x20a7:
case 0x20a8:
case 0x20a9:
case 0x20aa:
case 0x20ab:
case 0x20ac:
case 0x20ad:
case 0x20ae:
case 0x20af:
case 0x20b0:
case 0x20b1:
case 0x20b2:
case 0x20b3:
case 0x20b4:
case 0x20b5:
case 0x20b6:
case 0x20b7:
case 0x20b8:
case 0x20b9:
case 0x20ba:
case 0x20bb:
case 0x20bc:
case 0x20bd:
case 0x20be:
case 0x20bf:
case 0x20c0:
case 0x20c1:
case 0x20c2:
case 0x20c3:
case 0x20c4:
case 0x20c5:
case 0x20c6:
case 0x20c7:
case 0x20c8:
case 0x20c9:
case 0x20ca:
case 0x20cb:
case 0x20cc:
case 0x20cd:
case 0x20ce:
case 0x20cf:
case 0x20d0:
case 0x20d1:
case 0x20d2:
case 0x20d3:
case 0x20d4:
case 0x20d5:
case 0x20d6:
case 0x20d7:
case 0x20d8:
case 0x20d9:
case 0x20da:
case 0x20db:
case 0x20dc:
case 0x20dd:
case 0x20de:
case 0x20df:
case 0x20e0:
case 0x20e1:
case 0x20e2:
case 0x20e3:
case 0x20e4:
case 0x20e5:
case 0x20e6:
case 0x20e7:
case 0x20e8:
case 0x20e9:
case 0x20ea:
case 0x20eb:
case 0x20ec:
case 0x20ed:
case 0x20ee:
case 0x20ef:
case 0x20f0:
case 0x20f1:
case 0x20f2:
case 0x20f3:
case 0x20f4:
case 0x20f5:
case 0x20f6:
case 0x20f7:
case 0x20f8:
case 0x20f9:
case 0x20fa:
case 0x20fb:
case 0x20fc:
case 0x20fd:
case 0x20fe:
case 0x20ff:
case 0x2100:
case 0x2101:
case 0x2102:
case 0x2103:
case 0x2104:
case 0x2105:
case 0x2106:
case 0x2107:
case 0x2108:
case 0x2109:
case 0x210a:
case 0x210b:
case 0x210c:
case 0x210d:
case 0x210e:
case 0x210f:
case 0x2110:
case 0x2111:
case 0x2112:
case 0x2113:
case 0x2114:
case 0x2115:
case 0x2116:
case 0x2117:
case 0x2118:
case 0x2119:
case 0x211a:
case 0x211b:
case 0x211c:
case 0x211d:
case 0x211e:
case 0x211f:
case 0x2120:
case 0x2121:
case 0x2122:
case 0x2123:
case 0x2124:
case 0x2125:
case 0x2126:
case 0x2127:
case 0x2128:
case 0x2129:
case 0x212a:
case 0x212b:
case 0x212c:
case 0x212d:
case 0x212e:
case 0x212f:
case 0x2130:
case 0x2131:
case 0x2132:
case 0x2133:
case 0x2134:
case 0x2135:
case 0x2136:
case 0x2137:
case 0x2138:
case 0x2139:
case 0x213a:
case 0x213b:
case 0x213c:
case 0x213d:
case 0x213e:
case 0x213f:
case 0x2140:
case 0x2141:
case 0x2142:
case 0x2143:
case 0x2144:
case 0x2145:
case 0x2146:
case 0x2147:
case 0x2148:
case 0x2149:
case 0x214a:
case 0x214b:
case 0x214c:
case 0x214d:
case 0x214e:
case 0x214f:
case 0x2150:
case 0x2151:
case 0x2152:
case 0x2153:
case 0x2154:
case 0x2155:
case 0x2156:
case 0x2157:
case 0x2158:
case 0x2159:
case 0x215a:
case 0x215b:
case 0x215c:
case 0x215d:
case 0x215e:
case 0x215f:
case 0x2160:
case 0x2161:
case 0x2162:
case 0x2163:
case 0x2164:
case 0x2165:
case 0x2166:
case 0x2167:
case 0x2168:
case 0x2169:
case 0x216a:
case 0x216b:
case 0x216c:
case 0x216d:
case 0x216e:
case 0x216f:
case 0x2170:
case 0x2171:
case 0x2172:
case 0x2173:
case 0x2174:
case 0x2175:
case 0x2176:
case 0x2177:
case 0x2178:
case 0x2179:
case 0x217a:
case 0x217b:
case 0x217c:
case 0x217d:
case 0x217e:
case 0x217f:
case 0x2180:
case 0x2181:
case 0x2182:
case 0x2183:
case 0x2184:
case 0x2185:
case 0x2186:
case 0x2187:
case 0x2188:
case 0x2189:
case 0x218a:
case 0x218b:
case 0x218c:
case 0x218d:
case 0x218e:
case 0x218f:
case 0x2190:
case 0x2191:
case 0x2192:
case 0x2193:
case 0x2194:
case 0x2195:
case 0x2196:
case 0x2197:
case 0x2198:
case 0x2199:
case 0x219a:
case 0x219b:
case 0x219c:
case 0x219d:
case 0x219e:
case 0x219f:
case 0x21a0:
case 0x21a1:
case 0x21a2:
case 0x21a3:
case 0x21a4:
case 0x21a5:
case 0x21a6:
case 0x21a7:
case 0x21a8:
case 0x21a9:
case 0x21aa:
case 0x21ab:
case 0x21ac:
case 0x21ad:
case 0x21ae:
case 0x21af:
case 0x21b0:
case 0x21b1:
case 0x21b2:
case 0x21b3:
case 0x21b4:
case 0x21b5:
case 0x21b6:
case 0x21b7:
case 0x21b8:
case 0x21b9:
case 0x21ba:
case 0x21bb:
case 0x21bc:
case 0x21bd:
case 0x21be:
case 0x21bf:
case 0x21c0:
case 0x21c1:
case 0x21c2:
case 0x21c3:
case 0x21c4:
case 0x21c5:
case 0x21c6:
case 0x21c7:
case 0x21c8:
case 0x21c9:
case 0x21ca:
case 0x21cb:
case 0x21cc:
case 0x21cd:
case 0x21ce:
case 0x21cf:
case 0x21d0:
case 0x21d1:
case 0x21d2:
case 0x21d3:
case 0x21d4:
case 0x21d5:
case 0x21d6:
case 0x21d7:
case 0x21d8:
case 0x21d9:
case 0x21da:
case 0x21db:
case 0x21dc:
case 0x21dd:
case 0x21de:
case 0x21df:
case 0x21e0:
case 0x21e1:
case 0x21e2:
case 0x21e3:
case 0x21e4:
case 0x21e5:
case 0x21e6:
case 0x21e7:
case 0x21e8:
case 0x21e9:
case 0x21ea:
case 0x21eb:
case 0x21ec:
case 0x21ed:
case 0x21ee:
case 0x21ef:
case 0x21f0:
case 0x21f1:
case 0x21f2:
case 0x21f3:
case 0x21f4:
case 0x21f5:
case 0x21f6:
case 0x21f7:
case 0x21f8:
case 0x21f9:
case 0x21fa:
case 0x21fb:
case 0x21fc:
case 0x21fd:
case 0x21fe:
case 0x21ff:
case 0x2200:
case 0x2201:
case 0x2202:
case 0x2203:
case 0x2204:
case 0x2205:
case 0x2206:
case 0x2207:
case 0x2208:
case 0x2209:
case 0x220a:
case 0x220b:
case 0x220c:
case 0x220d:
case 0x220e:
case 0x220f:
case 0x2210:
case 0x2211:
case 0x2212:
case 0x2213:
case 0x2214:
case 0x2215:
case 0x2216:
case 0x2217:
case 0x2218:
case 0x2219:
case 0x221a:
case 0x221b:
case 0x221c:
case 0x221d:
case 0x221e:
case 0x221f:
case 0x2220:
case 0x2221:
case 0x2222:
case 0x2223:
case 0x2224:
case 0x2225:
case 0x2226:
case 0x2227:
case 0x2228:
case 0x2229:
case 0x222a:
case 0x222b:
case 0x222c:
case 0x222d:
case 0x222e:
case 0x222f:
case 0x2230:
case 0x2231:
case 0x2232:
case 0x2233:
case 0x2234:
case 0x2235:
case 0x2236:
case 0x2237:
case 0x2238:
case 0x2239:
case 0x223a:
case 0x223b:
case 0x223c:
case 0x223d:
case 0x223e:
case 0x223f:
case 0x2240:
case 0x2241:
case 0x2242:
case 0x2243:
case 0x2244:
case 0x2245:
case 0x2246:
case 0x2247:
case 0x2248:
case 0x2249:
case 0x224a:
case 0x224b:
case 0x224c:
case 0x224d:
case 0x224e:
case 0x224f:
case 0x2250:
case 0x2251:
case 0x2252:
case 0x2253:
case 0x2254:
case 0x2255:
case 0x2256:
case 0x2257:
case 0x2258:
case 0x2259:
case 0x225a:
case 0x225b:
case 0x225c:
case 0x225d:
case 0x225e:
case 0x225f:
case 0x2260:
case 0x2261:
case 0x2262:
case 0x2263:
case 0x2264:
case 0x2265:
case 0x2266:
case 0x2267:
case 0x2268:
case 0x2269:
case 0x226a:
case 0x226b:
case 0x226c:
case 0x226d:
case 0x226e:
case 0x226f:
case 0x2270:
case 0x2271:
case 0x2272:
case 0x2273:
case 0x2274:
case 0x2275:
case 0x2276:
case 0x2277:
case 0x2278:
case 0x2279:
case 0x227a:
case 0x227b:
case 0x227c:
case 0x227d:
case 0x227e:
case 0x227f:
case 0x2280:
case 0x2281:
case 0x2282:
case 0x2283:
case 0x2284:
case 0x2285:
case 0x2286:
case 0x2287:
case 0x2288:
case 0x2289:
case 0x228a:
case 0x228b:
case 0x228c:
case 0x228d:
case 0x228e:
case 0x228f:
case 0x2290:
case 0x2291:
case 0x2292:
case 0x2293:
case 0x2294:
case 0x2295:
case 0x2296:
case 0x2297:
case 0x2298:
case 0x2299:
case 0x229a:
case 0x229b:
case 0x229c:
case 0x229d:
case 0x229e:
case 0x229f:
case 0x22a0:
case 0x22a1:
case 0x22a2:
case 0x22a3:
case 0x22a4:
case 0x22a5:
case 0x22a6:
case 0x22a7:
case 0x22a8:
case 0x22a9:
case 0x22aa:
case 0x22ab:
case 0x22ac:
case 0x22ad:
case 0x22ae:
case 0x22af:
case 0x22b0:
case 0x22b1:
case 0x22b2:
case 0x22b3:
case 0x22b4:
case 0x22b5:
case 0x22b6:
case 0x22b7:
case 0x22b8:
case 0x22b9:
case 0x22ba:
case 0x22bb:
case 0x22bc:
case 0x22bd:
case 0x22be:
case 0x22bf:
case 0x22c0:
case 0x22c1:
case 0x22c2:
case 0x22c3:
case 0x22c4:
case 0x22c5:
case 0x22c6:
case 0x22c7:
case 0x22c8:
case 0x22c9:
case 0x22ca:
case 0x22cb:
case 0x22cc:
case 0x22cd:
case 0x22ce:
case 0x22cf:
case 0x22d0:
case 0x22d1:
case 0x22d2:
case 0x22d3:
case 0x22d4:
case 0x22d5:
case 0x22d6:
case 0x22d7:
case 0x22d8:
case 0x22d9:
case 0x22da:
case 0x22db:
case 0x22dc:
case 0x22dd:
case 0x22de:
case 0x22df:
case 0x22e0:
case 0x22e1:
case 0x22e2:
case 0x22e3:
case 0x22e4:
case 0x22e5:
case 0x22e6:
case 0x22e7:
case 0x22e8:
case 0x22e9:
case 0x22ea:
case 0x22eb:
case 0x22ec:
case 0x22ed:
case 0x22ee:
case 0x22ef:
case 0x22f0:
case 0x22f1:
case 0x22f2:
case 0x22f3:
case 0x22f4:
case 0x22f5:
case 0x22f6:
case 0x22f7:
case 0x22f8:
case 0x22f9:
case 0x22fa:
case 0x22fb:
case 0x22fc:
case 0x22fd:
case 0x22fe:
case 0x22ff:
case 0x2300:
case 0x2301:
case 0x2302:
case 0x2303:
case 0x2304:
case 0x2305:
case 0x2306:
case 0x2307:
case 0x2308:
case 0x2309:
case 0x230a:
case 0x230b:
case 0x230c:
case 0x230d:
case 0x230e:
case 0x230f:
case 0x2310:
case 0x2311:
case 0x2312:
case 0x2313:
case 0x2314:
case 0x2315:
case 0x2316:
case 0x2317:
case 0x2318:
case 0x2319:
case 0x231a:
case 0x231b:
case 0x231c:
case 0x231d:
case 0x231e:
case 0x231f:
case 0x2320:
case 0x2321:
case 0x2322:
case 0x2323:
case 0x2324:
case 0x2325:
case 0x2326:
case 0x2327:
case 0x2328:
case 0x2329:
case 0x232a:
case 0x232b:
case 0x232c:
case 0x232d:
case 0x232e:
case 0x232f:
case 0x2330:
case 0x2331:
case 0x2332:
case 0x2333:
case 0x2334:
case 0x2335:
case 0x2336:
case 0x2337:
case 0x2338:
case 0x2339:
case 0x233a:
case 0x233b:
case 0x233c:
case 0x233d:
case 0x233e:
case 0x233f:
case 0x2340:
case 0x2341:
case 0x2342:
case 0x2343:
case 0x2344:
case 0x2345:
case 0x2346:
case 0x2347:
case 0x2348:
case 0x2349:
case 0x234a:
case 0x234b:
case 0x234c:
case 0x234d:
case 0x234e:
case 0x234f:
case 0x2350:
case 0x2351:
case 0x2352:
case 0x2353:
case 0x2354:
case 0x2355:
case 0x2356:
case 0x2357:
case 0x2358:
case 0x2359:
case 0x235a:
case 0x235b:
case 0x235c:
case 0x235d:
case 0x235e:
case 0x235f:
case 0x2360:
case 0x2361:
case 0x2362:
case 0x2363:
case 0x2364:
case 0x2365:
case 0x2366:
case 0x2367:
case 0x2368:
case 0x2369:
case 0x236a:
case 0x236b:
case 0x236c:
case 0x236d:
case 0x236e:
case 0x236f:
case 0x2370:
case 0x2371:
case 0x2372:
case 0x2373:
case 0x2374:
case 0x2375:
case 0x2376:
case 0x2377:
case 0x2378:
case 0x2379:
case 0x237a:
case 0x237b:
case 0x237c:
case 0x237d:
case 0x237e:
case 0x237f:
case 0x2380:
case 0x2381:
case 0x2382:
case 0x2383:
case 0x2384:
case 0x2385:
case 0x2386:
case 0x2387:
case 0x2388:
case 0x2389:
case 0x238a:
case 0x238b:
case 0x238c:
case 0x238d:
case 0x238e:
case 0x238f:
case 0x2390:
case 0x2391:
case 0x2392:
case 0x2393:
case 0x2394:
case 0x2395:
case 0x2396:
case 0x2397:
case 0x2398:
case 0x2399:
case 0x239a:
case 0x239b:
case 0x239c:
case 0x239d:
case 0x239e:
case 0x239f:
case 0x23a0:
case 0x23a1:
case 0x23a2:
case 0x23a3:
case 0x23a4:
case 0x23a5:
case 0x23a6:
case 0x23a7:
case 0x23a8:
case 0x23a9:
case 0x23aa:
case 0x23ab:
case 0x23ac:
case 0x23ad:
case 0x23ae:
case 0x23af:
case 0x23b0:
case 0x23b1:
case 0x23b2:
case 0x23b3:
case 0x23b4:
case 0x23b5:
case 0x23b6:
case 0x23b7:
case 0x23b8:
case 0x23b9:
case 0x23ba:
case 0x23bb:
case 0x23bc:
case 0x23bd:
case 0x23be:
case 0x23bf:
case 0x23c0:
case 0x23c1:
case 0x23c2:
case 0x23c3:
case 0x23c4:
case 0x23c5:
case 0x23c6:
case 0x23c7:
case 0x23c8:
case 0x23c9:
case 0x23ca:
case 0x23cb:
case 0x23cc:
case 0x23cd:
case 0x23ce:
case 0x23cf:
case 0x23d0:
case 0x23d1:
case 0x23d2:
case 0x23d3:
case 0x23d4:
case 0x23d5:
case 0x23d6:
case 0x23d7:
case 0x23d8:
case 0x23d9:
case 0x23da:
case 0x23db:
case 0x23dc:
case 0x23dd:
case 0x23de:
case 0x23df:
case 0x23e0:
case 0x23e1:
case 0x23e2:
case 0x23e3:
case 0x23e4:
case 0x23e5:
case 0x23e6:
case 0x23e7:
case 0x23e8:
case 0x23e9:
case 0x23ea:
case 0x23eb:
case 0x23ec:
case 0x23ed:
case 0x23ee:
case 0x23ef:
case 0x23f0:
case 0x23f1:
case 0x23f2:
case 0x23f3:
case 0x23f4:
case 0x23f5:
case 0x23f6:
case 0x23f7:
case 0x23f8:
case 0x23f9:
case 0x23fa:
case 0x23fb:
case 0x23fc:
case 0x23fd:
case 0x23fe:
case 0x23ff:
case 0x2400:
case 0x2401:
case 0x2402:
case 0x2403:
case 0x2404:
case 0x2405:
case 0x2406:
case 0x2407:
case 0x2408:
case 0x2409:
case 0x240a:
case 0x240b:
case 0x240c:
case 0x240d:
case 0x240e:
case 0x240f:
case 0x2410:
case 0x2411:
case 0x2412:
case 0x2413:
case 0x2414:
case 0x2415:
case 0x2416:
case 0x2417:
case 0x2418:
case 0x2419:
case 0x241a:
case 0x241b:
case 0x241c:
case 0x241d:
case 0x241e:
case 0x241f:
case 0x2420:
case 0x2421:
case 0x2422:
case 0x2423:
case 0x2424:
case 0x2425:
case 0x2426:
case 0x2427:
case 0x2428:
case 0x2429:
case 0x242a:
case 0x242b:
case 0x242c:
case 0x242d:
case 0x242e:
case 0x242f:
case 0x2430:
case 0x2431:
case 0x2432:
case 0x2433:
case 0x2434:
case 0x2435:
case 0x2436:
case 0x2437:
case 0x2438:
case 0x2439:
case 0x243a:
case 0x243b:
case 0x243c:
case 0x243d:
case 0x243e:
case 0x243f:
case 0x2440:
case 0x2441:
case 0x2442:
case 0x2443:
case 0x2444:
case 0x2445:
case 0x2446:
case 0x2447:
case 0x2448:
case 0x2449:
case 0x244a:
case 0x244b:
case 0x244c:
case 0x244d:
case 0x244e:
case 0x244f:
case 0x2450:
case 0x2451:
case 0x2452:
case 0x2453:
case 0x2454:
case 0x2455:
case 0x2456:
case 0x2457:
case 0x2458:
case 0x2459:
case 0x245a:
case 0x245b:
case 0x245c:
case 0x245d:
case 0x245e:
case 0x245f:
case 0x2460:
case 0x2461:
case 0x2462:
case 0x2463:
case 0x2464:
case 0x2465:
case 0x2466:
case 0x2467:
case 0x2468:
case 0x2469:
case 0x246a:
case 0x246b:
case 0x246c:
case 0x246d:
case 0x246e:
case 0x246f:
case 0x2470:
case 0x2471:
case 0x2472:
case 0x2473:
case 0x2474:
case 0x2475:
case 0x2476:
case 0x2477:
case 0x2478:
case 0x2479:
case 0x247a:
case 0x247b:
case 0x247c:
case 0x247d:
case 0x247e:
case 0x247f:
case 0x2480:
case 0x2481:
case 0x2482:
case 0x2483:
case 0x2484:
case 0x2485:
case 0x2486:
case 0x2487:
case 0x2488:
case 0x2489:
case 0x248a:
case 0x248b:
case 0x248c:
case 0x248d:
case 0x248e:
case 0x248f:
case 0x2490:
case 0x2491:
case 0x2492:
case 0x2493:
case 0x2494:
case 0x2495:
case 0x2496:
case 0x2497:
case 0x2498:
case 0x2499:
case 0x249a:
case 0x249b:
case 0x249c:
case 0x249d:
case 0x249e:
case 0x249f:
case 0x24a0:
case 0x24a1:
case 0x24a2:
case 0x24a3:
case 0x24a4:
case 0x24a5:
case 0x24a6:
case 0x24a7:
case 0x24a8:
case 0x24a9:
case 0x24aa:
case 0x24ab:
case 0x24ac:
case 0x24ad:
case 0x24ae:
case 0x24af:
case 0x24b0:
case 0x24b1:
case 0x24b2:
case 0x24b3:
case 0x24b4:
case 0x24b5:
case 0x24b6:
case 0x24b7:
case 0x24b8:
case 0x24b9:
case 0x24ba:
case 0x24bb:
case 0x24bc:
case 0x24bd:
case 0x24be:
case 0x24bf:
case 0x24c0:
case 0x24c1:
case 0x24c2:
case 0x24c3:
case 0x24c4:
case 0x24c5:
case 0x24c6:
case 0x24c7:
case 0x24c8:
case 0x24c9:
case 0x24ca:
case 0x24cb:
case 0x24cc:
case 0x24cd:
case 0x24ce:
case 0x24cf:
case 0x24d0:
case 0x24d1:
case 0x24d2:
case 0x24d3:
case 0x24d4:
case 0x24d5:
case 0x24d6:
case 0x24d7:
case 0x24d8:
case 0x24d9:
case 0x24da:
case 0x24db:
case 0x24dc:
case 0x24dd:
case 0x24de:
case 0x24df:
case 0x24e0:
case 0x24e1:
case 0x24e2:
case 0x24e3:
case 0x24e4:
case 0x24e5:
case 0x24e6:
case 0x24e7:
case 0x24e8:
case 0x24e9:
case 0x24ea:
case 0x24eb:
case 0x24ec:
case 0x24ed:
case 0x24ee:
case 0x24ef:
case 0x24f0:
case 0x24f1:
case 0x24f2:
case 0x24f3:
case 0x24f4:
case 0x24f5:
case 0x24f6:
case 0x24f7:
case 0x24f8:
case 0x24f9:
case 0x24fa:
case 0x24fb:
case 0x24fc:
case 0x24fd:
case 0x24fe:
case 0x24ff:
case 0x2500:
case 0x2501:
case 0x2502:
case 0x2503:
case 0x2504:
case 0x2505:
case 0x2506:
case 0x2507:
case 0x2508:
case 0x2509:
case 0x250a:
case 0x250b:
case 0x250c:
case 0x250d:
case 0x250e:
case 0x250f:
case 0x2510:
case 0x2511:
case 0x2512:
case 0x2513:
case 0x2514:
case 0x2515:
case 0x2516:
case 0x2517:
case 0x2518:
case 0x2519:
case 0x251a:
case 0x251b:
case 0x251c:
case 0x251d:
case 0x251e:
case 0x251f:
case 0x2520:
case 0x2521:
case 0x2522:
case 0x2523:
case 0x2524:
case 0x2525:
case 0x2526:
case 0x2527:
case 0x2528:
case 0x2529:
case 0x252a:
case 0x252b:
case 0x252c:
case 0x252d:
case 0x252e:
case 0x252f:
case 0x2530:
case 0x2531:
case 0x2532:
case 0x2533:
case 0x2534:
case 0x2535:
case 0x2536:
case 0x2537:
case 0x2538:
case 0x2539:
case 0x253a:
case 0x253b:
case 0x253c:
case 0x253d:
case 0x253e:
case 0x253f:
case 0x2540:
case 0x2541:
case 0x2542:
case 0x2543:
case 0x2544:
case 0x2545:
case 0x2546:
case 0x2547:
case 0x2548:
case 0x2549:
case 0x254a:
case 0x254b:
case 0x254c:
case 0x254d:
case 0x254e:
case 0x254f:
case 0x2550:
case 0x2551:
case 0x2552:
case 0x2553:
case 0x2554:
case 0x2555:
case 0x2556:
case 0x2557:
case 0x2558:
case 0x2559:
case 0x255a:
case 0x255b:
case 0x255c:
case 0x255d:
case 0x255e:
case 0x255f:
case 0x2560:
case 0x2561:
case 0x2562:
case 0x2563:
case 0x2564:
case 0x2565:
case 0x2566:
case 0x2567:
case 0x2568:
case 0x2569:
case 0x256a:
case 0x256b:
case 0x256c:
case 0x256d:
case 0x256e:
case 0x256f:
case 0x2570:
case 0x2571:
case 0x2572:
case 0x2573:
case 0x2574:
case 0x2575:
case 0x2576:
case 0x2577:
case 0x2578:
case 0x2579:
case 0x257a:
case 0x257b:
case 0x257c:
case 0x257d:
case 0x257e:
case 0x257f:
case 0x2580:
case 0x2581:
case 0x2582:
case 0x2583:
case 0x2584:
case 0x2585:
case 0x2586:
case 0x2587:
case 0x2588:
case 0x2589:
case 0x258a:
case 0x258b:
case 0x258c:
case 0x258d:
case 0x258e:
case 0x258f:
case 0x2590:
case 0x2591:
case 0x2592:
case 0x2593:
case 0x2594:
case 0x2595:
case 0x2596:
case 0x2597:
case 0x2598:
case 0x2599:
case 0x259a:
case 0x259b:
case 0x259c:
case 0x259d:
case 0x259e:
case 0x259f:
case 0x25a0:
case 0x25a1:
case 0x25a2:
case 0x25a3:
case 0x25a4:
case 0x25a5:
case 0x25a6:
case 0x25a7:
case 0x25a8:
case 0x25a9:
case 0x25aa:
case 0x25ab:
case 0x25ac:
case 0x25ad:
case 0x25ae:
case 0x25af:
case 0x25b0:
case 0x25b1:
case 0x25b2:
case 0x25b3:
case 0x25b4:
case 0x25b5:
case 0x25b6:
case 0x25b7:
case 0x25b8:
case 0x25b9:
case 0x25ba:
case 0x25bb:
case 0x25bc:
case 0x25bd:
case 0x25be:
case 0x25bf:
case 0x25c0:
case 0x25c1:
case 0x25c2:
case 0x25c3:
case 0x25c4:
case 0x25c5:
case 0x25c6:
case 0x25c7:
case 0x25c8:
case 0x25c9:
case 0x25ca:
case 0x25cb:
case 0x25cc:
case 0x25cd:
case 0x25ce:
case 0x25cf:
case 0x25d0:
case 0x25d1:
case 0x25d2:
case 0x25d3:
case 0x25d4:
case 0x25d5:
case 0x25d6:
case 0x25d7:
case 0x25d8:
case 0x25d9:
case 0x25da:
case 0x25db:
case 0x25dc:
case 0x25dd:
case 0x25de:
case 0x25df:
case 0x25e0:
case 0x25e1:
case 0x25e2:
case 0x25e3:
case 0x25e4:
case 0x25e5:
case 0x25e6:
case 0x25e7:
case 0x25e8:
case 0x25e9:
case 0x25ea:
case 0x25eb:
case 0x25ec:
case 0x25ed:
case 0x25ee:
case 0x25ef:
case 0x25f0:
case 0x25f1:
case 0x25f2:
case 0x25f3:
case 0x25f4:
case 0x25f5:
case 0x25f6:
case 0x25f7:
case 0x25f8:
case 0x25f9:
case 0x25fa:
case 0x25fb:
case 0x25fc:
case 0x25fd:
case 0x25fe:
case 0x25ff:
case 0x2600:
case 0x2601:
case 0x2602:
case 0x2603:
case 0x2604:
case 0x2605:
case 0x2606:
case 0x2607:
case 0x2608:
case 0x2609:
case 0x260a:
case 0x260b:
case 0x260c:
case 0x260d:
case 0x260e:
case 0x260f:
case 0x2610:
case 0x2611:
case 0x2612:
case 0x2613:
case 0x2614:
case 0x2615:
case 0x2616:
case 0x2617:
case 0x2618:
case 0x2619:
case 0x261a:
case 0x261b:
case 0x261c:
case 0x261d:
case 0x261e:
case 0x261f:
case 0x2620:
case 0x2621:
case 0x2622:
case 0x2623:
case 0x2624:
case 0x2625:
case 0x2626:
case 0x2627:
case 0x2628:
case 0x2629:
case 0x262a:
case 0x262b:
case 0x262c:
case 0x262d:
case 0x262e:
case 0x262f:
case 0x2630:
case 0x2631:
case 0x2632:
case 0x2633:
case 0x2634:
case 0x2635:
case 0x2636:
case 0x2637:
case 0x2638:
case 0x2639:
case 0x263a:
case 0x263b:
case 0x263c:
case 0x263d:
case 0x263e:
case 0x263f:
case 0x2640:
case 0x2641:
case 0x2642:
case 0x2643:
case 0x2644:
case 0x2645:
case 0x2646:
case 0x2647:
case 0x2648:
case 0x2649:
case 0x264a:
case 0x264b:
case 0x264c:
case 0x264d:
case 0x264e:
case 0x264f:
case 0x2650:
case 0x2651:
case 0x2652:
case 0x2653:
case 0x2654:
case 0x2655:
case 0x2656:
case 0x2657:
case 0x2658:
case 0x2659:
case 0x265a:
case 0x265b:
case 0x265c:
case 0x265d:
case 0x265e:
case 0x265f:
case 0x2660:
case 0x2661:
case 0x2662:
case 0x2663:
case 0x2664:
case 0x2665:
case 0x2666:
case 0x2667:
case 0x2668:
case 0x2669:
case 0x266a:
case 0x266b:
case 0x266c:
case 0x266d:
case 0x266e:
case 0x266f:
case 0x2670:
case 0x2671:
case 0x2672:
case 0x2673:
case 0x2674:
case 0x2675:
case 0x2676:
case 0x2677:
case 0x2678:
case 0x2679:
case 0x267a:
case 0x267b:
case 0x267c:
case 0x267d:
case 0x267e:
case 0x267f:
case 0x2680:
case 0x2681:
case 0x2682:
case 0x2683:
case 0x2684:
case 0x2685:
case 0x2686:
case 0x2687:
case 0x2688:
case 0x2689:
case 0x268a:
case 0x268b:
case 0x268c:
case 0x268d:
case 0x268e:
case 0x268f:
case 0x2690:
case 0x2691:
case 0x2692:
case 0x2693:
case 0x2694:
case 0x2695:
case 0x2696:
case 0x2697:
case 0x2698:
case 0x2699:
case 0x269a:
case 0x269b:
case 0x269c:
case 0x269d:
case 0x269e:
case 0x269f:
case 0x26a0:
case 0x26a1:
case 0x26a2:
case 0x26a3:
case 0x26a4:
case 0x26a5:
case 0x26a6:
case 0x26a7:
case 0x26a8:
case 0x26a9:
case 0x26aa:
case 0x26ab:
case 0x26ac:
case 0x26ad:
case 0x26ae:
case 0x26af:
case 0x26b0:
case 0x26b1:
case 0x26b2:
case 0x26b3:
case 0x26b4:
case 0x26b5:
case 0x26b6:
case 0x26b7:
case 0x26b8:
case 0x26b9:
case 0x26ba:
case 0x26bb:
case 0x26bc:
case 0x26bd:
case 0x26be:
case 0x26bf:
case 0x26c0:
case 0x26c1:
case 0x26c2:
case 0x26c3:
case 0x26c4:
case 0x26c5:
case 0x26c6:
case 0x26c7:
case 0x26c8:
case 0x26c9:
case 0x26ca:
case 0x26cb:
case 0x26cc:
case 0x26cd:
case 0x26ce:
case 0x26cf:
case 0x26d0:
case 0x26d1:
case 0x26d2:
case 0x26d3:
case 0x26d4:
case 0x26d5:
case 0x26d6:
case 0x26d7:
case 0x26d8:
case 0x26d9:
case 0x26da:
case 0x26db:
case 0x26dc:
case 0x26dd:
case 0x26de:
case 0x26df:
case 0x26e0:
case 0x26e1:
case 0x26e2:
case 0x26e3:
case 0x26e4:
case 0x26e5:
case 0x26e6:
case 0x26e7:
case 0x26e8:
case 0x26e9:
case 0x26ea:
case 0x26eb:
case 0x26ec:
case 0x26ed:
case 0x26ee:
case 0x26ef:
case 0x26f0:
case 0x26f1:
case 0x26f2:
case 0x26f3:
case 0x26f4:
case 0x26f5:
case 0x26f6:
case 0x26f7:
case 0x26f8:
case 0x26f9:
case 0x26fa:
case 0x26fb:
case 0x26fc:
case 0x26fd:
case 0x26fe:
case 0x26ff:
case 0x2700:
case 0x2701:
case 0x2702:
case 0x2703:
case 0x2704:
case 0x2705:
case 0x2706:
case 0x2707:
case 0x2708:
case 0x2709:
case 0x270a:
case 0x270b:
case 0x270c:
case 0x270d:
case 0x270e:
case 0x270f:
case 0x2710:
case 0x2711:
case 0x2712:
case 0x2713:
case 0x2714:
case 0x2715:
case 0x2716:
case 0x2717:
case 0x2718:
case 0x2719:
case 0x271a:
case 0x271b:
case 0x271c:
case 0x271d:
case 0x271e:
case 0x271f:
case 0x2720:
case 0x2721:
case 0x2722:
case 0x2723:
case 0x2724:
case 0x2725:
case 0x2726:
case 0x2727:
case 0x2728:
case 0x2729:
case 0x272a:
case 0x272b:
case 0x272c:
case 0x272d:
case 0x272e:
case 0x272f:
case 0x2730:
case 0x2731:
case 0x2732:
case 0x2733:
case 0x2734:
case 0x2735:
case 0x2736:
case 0x2737:
case 0x2738:
case 0x2739:
case 0x273a:
case 0x273b:
case 0x273c:
case 0x273d:
case 0x273e:
case 0x273f:
case 0x2740:
case 0x2741:
case 0x2742:
case 0x2743:
case 0x2744:
case 0x2745:
case 0x2746:
case 0x2747:
case 0x2748:
case 0x2749:
case 0x274a:
case 0x274b:
case 0x274c:
case 0x274d:
case 0x274e:
case 0x274f:
case 0x2750:
case 0x2751:
case 0x2752:
case 0x2753:
case 0x2754:
case 0x2755:
case 0x2756:
case 0x2757:
case 0x2758:
case 0x2759:
case 0x275a:
case 0x275b:
case 0x275c:
case 0x275d:
case 0x275e:
case 0x275f:
case 0x2760:
case 0x2761:
case 0x2762:
case 0x2763:
case 0x2764:
case 0x2765:
case 0x2766:
case 0x2767:
case 0x2768:
case 0x2769:
case 0x276a:
case 0x276b:
case 0x276c:
case 0x276d:
case 0x276e:
case 0x276f:
case 0x2770:
case 0x2771:
case 0x2772:
case 0x2773:
case 0x2774:
case 0x2775:
case 0x2776:
case 0x2777:
case 0x2778:
case 0x2779:
case 0x277a:
case 0x277b:
case 0x277c:
case 0x277d:
case 0x277e:
case 0x277f:
case 0x2780:
case 0x2781:
case 0x2782:
case 0x2783:
case 0x2784:
case 0x2785:
case 0x2786:
case 0x2787:
case 0x2788:
case 0x2789:
case 0x278a:
case 0x278b:
case 0x278c:
case 0x278d:
case 0x278e:
case 0x278f:
case 0x2790:
case 0x2791:
case 0x2792:
case 0x2793:
case 0x2794:
case 0x2795:
case 0x2796:
case 0x2797:
case 0x2798:
case 0x2799:
case 0x279a:
case 0x279b:
case 0x279c:
case 0x279d:
case 0x279e:
case 0x279f:
case 0x27a0:
case 0x27a1:
case 0x27a2:
case 0x27a3:
case 0x27a4:
case 0x27a5:
case 0x27a6:
case 0x27a7:
case 0x27a8:
case 0x27a9:
case 0x27aa:
case 0x27ab:
case 0x27ac:
case 0x27ad:
case 0x27ae:
case 0x27af:
case 0x27b0:
case 0x27b1:
case 0x27b2:
case 0x27b3:
case 0x27b4:
case 0x27b5:
case 0x27b6:
case 0x27b7:
case 0x27b8:
case 0x27b9:
case 0x27ba:
case 0x27bb:
case 0x27bc:
case 0x27bd:
case 0x27be:
case 0x27bf:
case 0x27c0:
case 0x27c1:
case 0x27c2:
case 0x27c3:
case 0x27c4:
case 0x27c5:
case 0x27c6:
case 0x27c7:
case 0x27c8:
case 0x27c9:
case 0x27ca:
case 0x27cb:
case 0x27cc:
case 0x27cd:
case 0x27ce:
case 0x27cf:
case 0x27d0:
case 0x27d1:
case 0x27d2:
case 0x27d3:
case 0x27d4:
case 0x27d5:
case 0x27d6:
case 0x27d7:
case 0x27d8:
case 0x27d9:
case 0x27da:
case 0x27db:
case 0x27dc:
case 0x27dd:
case 0x27de:
case 0x27df:
case 0x27e0:
case 0x27e1:
case 0x27e2:
case 0x27e3:
case 0x27e4:
case 0x27e5:
case 0x27e6:
case 0x27e7:
case 0x27e8:
case 0x27e9:
case 0x27ea:
case 0x27eb:
case 0x27ec:
case 0x27ed:
case 0x27ee:
case 0x27ef:
case 0x27f0:
case 0x27f1:
case 0x27f2:
case 0x27f3:
case 0x27f4:
case 0x27f5:
case 0x27f6:
case 0x27f7:
case 0x27f8:
case 0x27f9:
case 0x27fa:
case 0x27fb:
case 0x27fc:
case 0x27fd:
case 0x27fe:
case 0x27ff:
case 0x2800:
case 0x2801:
case 0x2802:
case 0x2803:
case 0x2804:
case 0x2805:
case 0x2806:
case 0x2807:
case 0x2808:
case 0x2809:
case 0x280a:
case 0x280b:
case 0x280c:
case 0x280d:
case 0x280e:
case 0x280f:
case 0x2810:
case 0x2811:
case 0x2812:
case 0x2813:
case 0x2814:
case 0x2815:
case 0x2816:
case 0x2817:
case 0x2818:
case 0x2819:
case 0x281a:
case 0x281b:
case 0x281c:
case 0x281d:
case 0x281e:
case 0x281f:
case 0x2820:
case 0x2821:
case 0x2822:
case 0x2823:
case 0x2824:
case 0x2825:
case 0x2826:
case 0x2827:
case 0x2828:
case 0x2829:
case 0x282a:
case 0x282b:
case 0x282c:
case 0x282d:
case 0x282e:
case 0x282f:
case 0x2830:
case 0x2831:
case 0x2832:
case 0x2833:
case 0x2834:
case 0x2835:
case 0x2836:
case 0x2837:
case 0x2838:
case 0x2839:
case 0x283a:
case 0x283b:
case 0x283c:
case 0x283d:
case 0x283e:
case 0x283f:
case 0x2840:
case 0x2841:
case 0x2842:
case 0x2843:
case 0x2844:
case 0x2845:
case 0x2846:
case 0x2847:
case 0x2848:
case 0x2849:
case 0x284a:
case 0x284b:
case 0x284c:
case 0x284d:
case 0x284e:
case 0x284f:
case 0x2850:
case 0x2851:
case 0x2852:
case 0x2853:
case 0x2854:
case 0x2855:
case 0x2856:
case 0x2857:
case 0x2858:
case 0x2859:
case 0x285a:
case 0x285b:
case 0x285c:
case 0x285d:
case 0x285e:
case 0x285f:
case 0x2860:
case 0x2861:
case 0x2862:
case 0x2863:
case 0x2864:
case 0x2865:
case 0x2866:
case 0x2867:
case 0x2868:
case 0x2869:
case 0x286a:
case 0x286b:
case 0x286c:
case 0x286d:
case 0x286e:
case 0x286f:
case 0x2870:
case 0x2871:
case 0x2872:
case 0x2873:
case 0x2874:
case 0x2875:
case 0x2876:
case 0x2877:
case 0x2878:
case 0x2879:
case 0x287a:
case 0x287b:
case 0x287c:
case 0x287d:
case 0x287e:
case 0x287f:
case 0x2880:
case 0x2881:
case 0x2882:
case 0x2883:
case 0x2884:
case 0x2885:
case 0x2886:
case 0x2887:
case 0x2888:
case 0x2889:
case 0x288a:
case 0x288b:
case 0x288c:
case 0x288d:
case 0x288e:
case 0x288f:
case 0x2890:
case 0x2891:
case 0x2892:
case 0x2893:
case 0x2894:
case 0x2895:
case 0x2896:
case 0x2897:
case 0x2898:
case 0x2899:
case 0x289a:
case 0x289b:
case 0x289c:
case 0x289d:
case 0x289e:
case 0x289f:
case 0x28a0:
case 0x28a1:
case 0x28a2:
case 0x28a3:
case 0x28a4:
case 0x28a5:
case 0x28a6:
case 0x28a7:
case 0x28a8:
case 0x28a9:
case 0x28aa:
case 0x28ab:
case 0x28ac:
case 0x28ad:
case 0x28ae:
case 0x28af:
case 0x28b0:
case 0x28b1:
case 0x28b2:
case 0x28b3:
case 0x28b4:
case 0x28b5:
case 0x28b6:
case 0x28b7:
case 0x28b8:
case 0x28b9:
case 0x28ba:
case 0x28bb:
case 0x28bc:
case 0x28bd:
case 0x28be:
case 0x28bf:
case 0x28c0:
case 0x28c1:
case 0x28c2:
case 0x28c3:
case 0x28c4:
case 0x28c5:
case 0x28c6:
case 0x28c7:
case 0x28c8:
case 0x28c9:
case 0x28ca:
case 0x28cb:
case 0x28cc:
case 0x28cd:
case 0x28ce:
case 0x28cf:
case 0x28d0:
case 0x28d1:
case 0x28d2:
case 0x28d3:
case 0x28d4:
case 0x28d5:
case 0x28d6:
case 0x28d7:
case 0x28d8:
case 0x28d9:
case 0x28da:
case 0x28db:
case 0x28dc:
case 0x28dd:
case 0x28de:
case 0x28df:
case 0x28e0:
case 0x28e1:
case 0x28e2:
case 0x28e3:
case 0x28e4:
case 0x28e5:
case 0x28e6:
case 0x28e7:
case 0x28e8:
case 0x28e9:
case 0x28ea:
case 0x28eb:
case 0x28ec:
case 0x28ed:
case 0x28ee:
case 0x28ef:
case 0x28f0:
case 0x28f1:
case 0x28f2:
case 0x28f3:
case 0x28f4:
case 0x28f5:
case 0x28f6:
case 0x28f7:
case 0x28f8:
case 0x28f9:
case 0x28fa:
case 0x28fb:
case 0x28fc:
case 0x28fd:
case 0x28fe:
case 0x28ff:
case 0x2900:
case 0x2901:
case 0x2902:
case 0x2903:
case 0x2904:
case 0x2905:
case 0x2906:
case 0x2907:
case 0x2908:
case 0x2909:
case 0x290a:
case 0x290b:
case 0x290c:
case 0x290d:
case 0x290e:
case 0x290f:
case 0x2910:
case 0x2911:
case 0x2912:
case 0x2913:
case 0x2914:
case 0x2915:
case 0x2916:
case 0x2917:
case 0x2918:
case 0x2919:
case 0x291a:
case 0x291b:
case 0x291c:
case 0x291d:
case 0x291e:
case 0x291f:
case 0x2920:
case 0x2921:
case 0x2922:
case 0x2923:
case 0x2924:
case 0x2925:
case 0x2926:
case 0x2927:
case 0x2928:
case 0x2929:
case 0x292a:
case 0x292b:
case 0x292c:
case 0x292d:
case 0x292e:
case 0x292f:
case 0x2930:
case 0x2931:
case 0x2932:
case 0x2933:
case 0x2934:
case 0x2935:
case 0x2936:
case 0x2937:
case 0x2938:
case 0x2939:
case 0x293a:
case 0x293b:
case 0x293c:
case 0x293d:
case 0x293e:
case 0x293f:
case 0x2940:
case 0x2941:
case 0x2942:
case 0x2943:
case 0x2944:
case 0x2945:
case 0x2946:
case 0x2947:
case 0x2948:
case 0x2949:
case 0x294a:
case 0x294b:
case 0x294c:
case 0x294d:
case 0x294e:
case 0x294f:
case 0x2950:
case 0x2951:
case 0x2952:
case 0x2953:
case 0x2954:
case 0x2955:
case 0x2956:
case 0x2957:
case 0x2958:
case 0x2959:
case 0x295a:
case 0x295b:
case 0x295c:
case 0x295d:
case 0x295e:
case 0x295f:
case 0x2960:
case 0x2961:
case 0x2962:
case 0x2963:
case 0x2964:
case 0x2965:
case 0x2966:
case 0x2967:
case 0x2968:
case 0x2969:
case 0x296a:
case 0x296b:
case 0x296c:
case 0x296d:
case 0x296e:
case 0x296f:
case 0x2970:
case 0x2971:
case 0x2972:
case 0x2973:
case 0x2974:
case 0x2975:
case 0x2976:
case 0x2977:
case 0x2978:
case 0x2979:
case 0x297a:
case 0x297b:
case 0x297c:
case 0x297d:
case 0x297e:
case 0x297f:
case 0x2980:
case 0x2981:
case 0x2982:
case 0x2983:
case 0x2984:
case 0x2985:
case 0x2986:
case 0x2987:
case 0x2988:
case 0x2989:
case 0x298a:
case 0x298b:
case 0x298c:
case 0x298d:
case 0x298e:
case 0x298f:
case 0x2990:
case 0x2991:
case 0x2992:
case 0x2993:
case 0x2994:
case 0x2995:
case 0x2996:
case 0x2997:
case 0x2998:
case 0x2999:
case 0x299a:
case 0x299b:
case 0x299c:
case 0x299d:
case 0x299e:
case 0x299f:
case 0x29a0:
case 0x29a1:
case 0x29a2:
case 0x29a3:
case 0x29a4:
case 0x29a5:
case 0x29a6:
case 0x29a7:
case 0x29a8:
case 0x29a9:
case 0x29aa:
case 0x29ab:
case 0x29ac:
case 0x29ad:
case 0x29ae:
case 0x29af:
case 0x29b0:
case 0x29b1:
case 0x29b2:
case 0x29b3:
case 0x29b4:
case 0x29b5:
case 0x29b6:
case 0x29b7:
case 0x29b8:
case 0x29b9:
case 0x29ba:
case 0x29bb:
case 0x29bc:
case 0x29bd:
case 0x29be:
case 0x29bf:
case 0x29c0:
case 0x29c1:
case 0x29c2:
case 0x29c3:
case 0x29c4:
case 0x29c5:
case 0x29c6:
case 0x29c7:
case 0x29c8:
case 0x29c9:
case 0x29ca:
case 0x29cb:
case 0x29cc:
case 0x29cd:
case 0x29ce:
case 0x29cf:
case 0x29d0:
case 0x29d1:
case 0x29d2:
case 0x29d3:
case 0x29d4:
case 0x29d5:
case 0x29d6:
case 0x29d7:
case 0x29d8:
case 0x29d9:
case 0x29da:
case 0x29db:
case 0x29dc:
case 0x29dd:
case 0x29de:
case 0x29df:
case 0x29e0:
case 0x29e1:
case 0x29e2:
case 0x29e3:
case 0x29e4:
case 0x29e5:
case 0x29e6:
case 0x29e7:
case 0x29e8:
case 0x29e9:
case 0x29ea:
case 0x29eb:
case 0x29ec:
case 0x29ed:
case 0x29ee:
case 0x29ef:
case 0x29f0:
case 0x29f1:
case 0x29f2:
case 0x29f3:
case 0x29f4:
case 0x29f5:
case 0x29f6:
case 0x29f7:
case 0x29f8:
case 0x29f9:
case 0x29fa:
case 0x29fb:
case 0x29fc:
case 0x29fd:
case 0x29fe:
case 0x29ff:
case 0x2a00:
case 0x2a01:
case 0x2a02:
case 0x2a03:
case 0x2a04:
case 0x2a05:
case 0x2a06:
case 0x2a07:
case 0x2a08:
case 0x2a09:
case 0x2a0a:
case 0x2a0b:
case 0x2a0c:
case 0x2a0d:
case 0x2a0e:
case 0x2a0f:
case 0x2a10:
case 0x2a11:
case 0x2a12:
case 0x2a13:
case 0x2a14:
case 0x2a15:
case 0x2a16:
case 0x2a17:
case 0x2a18:
case 0x2a19:
case 0x2a1a:
case 0x2a1b:
case 0x2a1c:
case 0x2a1d:
case 0x2a1e:
case 0x2a1f:
case 0x2a20:
case 0x2a21:
case 0x2a22:
case 0x2a23:
case 0x2a24:
case 0x2a25:
case 0x2a26:
case 0x2a27:
case 0x2a28:
case 0x2a29:
case 0x2a2a:
case 0x2a2b:
case 0x2a2c:
case 0x2a2d:
case 0x2a2e:
case 0x2a2f:
case 0x2a30:
case 0x2a31:
case 0x2a32:
case 0x2a33:
case 0x2a34:
case 0x2a35:
case 0x2a36:
case 0x2a37:
case 0x2a38:
case 0x2a39:
case 0x2a3a:
case 0x2a3b:
case 0x2a3c:
case 0x2a3d:
case 0x2a3e:
case 0x2a3f:
case 0x2a40:
case 0x2a41:
case 0x2a42:
case 0x2a43:
case 0x2a44:
case 0x2a45:
case 0x2a46:
case 0x2a47:
case 0x2a48:
case 0x2a49:
case 0x2a4a:
case 0x2a4b:
case 0x2a4c:
case 0x2a4d:
case 0x2a4e:
case 0x2a4f:
case 0x2a50:
case 0x2a51:
case 0x2a52:
case 0x2a53:
case 0x2a54:
case 0x2a55:
case 0x2a56:
case 0x2a57:
case 0x2a58:
case 0x2a59:
case 0x2a5a:
case 0x2a5b:
case 0x2a5c:
case 0x2a5d:
case 0x2a5e:
case 0x2a5f:
case 0x2a60:
case 0x2a61:
case 0x2a62:
case 0x2a63:
case 0x2a64:
case 0x2a65:
case 0x2a66:
case 0x2a67:
case 0x2a68:
case 0x2a69:
case 0x2a6a:
case 0x2a6b:
case 0x2a6c:
case 0x2a6d:
case 0x2a6e:
case 0x2a6f:
case 0x2a70:
case 0x2a71:
case 0x2a72:
case 0x2a73:
case 0x2a74:
case 0x2a75:
case 0x2a76:
case 0x2a77:
case 0x2a78:
case 0x2a79:
case 0x2a7a:
case 0x2a7b:
case 0x2a7c:
case 0x2a7d:
case 0x2a7e:
case 0x2a7f:
case 0x2a80:
case 0x2a81:
case 0x2a82:
case 0x2a83:
case 0x2a84:
case 0x2a85:
case 0x2a86:
case 0x2a87:
case 0x2a88:
case 0x2a89:
case 0x2a8a:
case 0x2a8b:
case 0x2a8c:
case 0x2a8d:
case 0x2a8e:
case 0x2a8f:
case 0x2a90:
case 0x2a91:
case 0x2a92:
case 0x2a93:
case 0x2a94:
case 0x2a95:
case 0x2a96:
case 0x2a97:
case 0x2a98:
case 0x2a99:
case 0x2a9a:
case 0x2a9b:
case 0x2a9c:
case 0x2a9d:
case 0x2a9e:
case 0x2a9f:
case 0x2aa0:
case 0x2aa1:
case 0x2aa2:
case 0x2aa3:
case 0x2aa4:
case 0x2aa5:
case 0x2aa6:
case 0x2aa7:
case 0x2aa8:
case 0x2aa9:
case 0x2aaa:
case 0x2aab:
case 0x2aac:
case 0x2aad:
case 0x2aae:
case 0x2aaf:
case 0x2ab0:
case 0x2ab1:
case 0x2ab2:
case 0x2ab3:
case 0x2ab4:
case 0x2ab5:
case 0x2ab6:
case 0x2ab7:
case 0x2ab8:
case 0x2ab9:
case 0x2aba:
case 0x2abb:
case 0x2abc:
case 0x2abd:
case 0x2abe:
case 0x2abf:
case 0x2ac0:
case 0x2ac1:
case 0x2ac2:
case 0x2ac3:
case 0x2ac4:
case 0x2ac5:
case 0x2ac6:
case 0x2ac7:
case 0x2ac8:
case 0x2ac9:
case 0x2aca:
case 0x2acb:
case 0x2acc:
case 0x2acd:
case 0x2ace:
case 0x2acf:
case 0x2ad0:
case 0x2ad1:
case 0x2ad2:
case 0x2ad3:
case 0x2ad4:
case 0x2ad5:
case 0x2ad6:
case 0x2ad7:
case 0x2ad8:
case 0x2ad9:
case 0x2ada:
case 0x2adb:
case 0x2adc:
case 0x2add:
case 0x2ade:
case 0x2adf:
case 0x2ae0:
case 0x2ae1:
case 0x2ae2:
case 0x2ae3:
case 0x2ae4:
case 0x2ae5:
case 0x2ae6:
case 0x2ae7:
case 0x2ae8:
case 0x2ae9:
case 0x2aea:
case 0x2aeb:
case 0x2aec:
case 0x2aed:
case 0x2aee:
case 0x2aef:
case 0x2af0:
case 0x2af1:
case 0x2af2:
case 0x2af3:
case 0x2af4:
case 0x2af5:
case 0x2af6:
case 0x2af7:
case 0x2af8:
case 0x2af9:
case 0x2afa:
case 0x2afb:
case 0x2afc:
case 0x2afd:
case 0x2afe:
case 0x2aff:
case 0x2b00:
case 0x2b01:
case 0x2b02:
case 0x2b03:
case 0x2b04:
case 0x2b05:
case 0x2b06:
case 0x2b07:
case 0x2b08:
case 0x2b09:
case 0x2b0a:
case 0x2b0b:
case 0x2b0c:
case 0x2b0d:
case 0x2b0e:
case 0x2b0f:
case 0x2b10:
case 0x2b11:
case 0x2b12:
case 0x2b13:
case 0x2b14:
case 0x2b15:
case 0x2b16:
case 0x2b17:
case 0x2b18:
case 0x2b19:
case 0x2b1a:
case 0x2b1b:
case 0x2b1c:
case 0x2b1d:
case 0x2b1e:
case 0x2b1f:
case 0x2b20:
case 0x2b21:
case 0x2b22:
case 0x2b23:
case 0x2b24:
case 0x2b25:
case 0x2b26:
case 0x2b27:
case 0x2b28:
case 0x2b29:
case 0x2b2a:
case 0x2b2b:
case 0x2b2c:
case 0x2b2d:
case 0x2b2e:
case 0x2b2f:
case 0x2b30:
case 0x2b31:
case 0x2b32:
case 0x2b33:
case 0x2b34:
case 0x2b35:
case 0x2b36:
case 0x2b37:
case 0x2b38:
case 0x2b39:
case 0x2b3a:
case 0x2b3b:
case 0x2b3c:
case 0x2b3d:
case 0x2b3e:
case 0x2b3f:
case 0x2b40:
case 0x2b41:
case 0x2b42:
case 0x2b43:
case 0x2b44:
case 0x2b45:
case 0x2b46:
case 0x2b47:
case 0x2b48:
case 0x2b49:
case 0x2b4a:
case 0x2b4b:
case 0x2b4c:
case 0x2b4d:
case 0x2b4e:
case 0x2b4f:
case 0x2b50:
case 0x2b51:
case 0x2b52:
case 0x2b53:
case 0x2b54:
case 0x2b55:
case 0x2b56:
case 0x2b57:
case 0x2b58:
case 0x2b59:
case 0x2b5a:
case 0x2b5b:
case 0x2b5c:
case 0x2b5d:
case 0x2b5e:
case 0x2b5f:
case 0x2b60:
case 0x2b61:
case 0x2b62:
case 0x2b63:
case 0x2b64:
case 0x2b65:
case 0x2b66:
case 0x2b67:
case 0x2b68:
case 0x2b69:
case 0x2b6a:
case 0x2b6b:
case 0x2b6c:
case 0x2b6d:
case 0x2b6e:
case 0x2b6f:
case 0x2b70:
case 0x2b71:
case 0x2b72:
case 0x2b73:
case 0x2b74:
case 0x2b75:
case 0x2b76:
case 0x2b77:
case 0x2b78:
case 0x2b79:
case 0x2b7a:
case 0x2b7b:
case 0x2b7c:
case 0x2b7d:
case 0x2b7e:
case 0x2b7f:
case 0x2b80:
case 0x2b81:
case 0x2b82:
case 0x2b83:
case 0x2b84:
case 0x2b85:
case 0x2b86:
case 0x2b87:
case 0x2b88:
case 0x2b89:
case 0x2b8a:
case 0x2b8b:
case 0x2b8c:
case 0x2b8d:
case 0x2b8e:
case 0x2b8f:
case 0x2b90:
case 0x2b91:
case 0x2b92:
case 0x2b93:
case 0x2b94:
case 0x2b95:
case 0x2b96:
case 0x2b97:
case 0x2b98:
case 0x2b99:
case 0x2b9a:
case 0x2b9b:
case 0x2b9c:
case 0x2b9d:
case 0x2b9e:
case 0x2b9f:
case 0x2ba0:
case 0x2ba1:
case 0x2ba2:
case 0x2ba3:
case 0x2ba4:
case 0x2ba5:
case 0x2ba6:
case 0x2ba7:
case 0x2ba8:
case 0x2ba9:
case 0x2baa:
case 0x2bab:
case 0x2bac:
case 0x2bad:
case 0x2bae:
case 0x2baf:
case 0x2bb0:
case 0x2bb1:
case 0x2bb2:
case 0x2bb3:
case 0x2bb4:
case 0x2bb5:
case 0x2bb6:
case 0x2bb7:
case 0x2bb8:
case 0x2bb9:
case 0x2bba:
case 0x2bbb:
case 0x2bbc:
case 0x2bbd:
case 0x2bbe:
case 0x2bbf:
case 0x2bc0:
case 0x2bc1:
case 0x2bc2:
case 0x2bc3:
case 0x2bc4:
case 0x2bc5:
case 0x2bc6:
case 0x2bc7:
case 0x2bc8:
case 0x2bc9:
case 0x2bca:
case 0x2bcb:
case 0x2bcc:
case 0x2bcd:
case 0x2bce:
case 0x2bcf:
case 0x2bd0:
case 0x2bd1:
case 0x2bd2:
case 0x2bd3:
case 0x2bd4:
case 0x2bd5:
case 0x2bd6:
case 0x2bd7:
case 0x2bd8:
case 0x2bd9:
case 0x2bda:
case 0x2bdb:
case 0x2bdc:
case 0x2bdd:
case 0x2bde:
case 0x2bdf:
case 0x2be0:
case 0x2be1:
case 0x2be2:
case 0x2be3:
case 0x2be4:
case 0x2be5:
case 0x2be6:
case 0x2be7:
case 0x2be8:
case 0x2be9:
case 0x2bea:
case 0x2beb:
case 0x2bec:
case 0x2bed:
case 0x2bee:
case 0x2bef:
case 0x2bf0:
case 0x2bf1:
case 0x2bf2:
case 0x2bf3:
case 0x2bf4:
case 0x2bf5:
case 0x2bf6:
case 0x2bf7:
case 0x2bf8:
case 0x2bf9:
case 0x2bfa:
case 0x2bfb:
case 0x2bfc:
case 0x2bfd:
case 0x2bfe:
case 0x2bff:
case 0x2c00:
case 0x2c01:
case 0x2c02:
case 0x2c03:
case 0x2c04:
case 0x2c05:
case 0x2c06:
case 0x2c07:
case 0x2c08:
case 0x2c09:
case 0x2c0a:
case 0x2c0b:
case 0x2c0c:
case 0x2c0d:
case 0x2c0e:
case 0x2c0f:
case 0x2c10:
case 0x2c11:
case 0x2c12:
case 0x2c13:
case 0x2c14:
case 0x2c15:
case 0x2c16:
case 0x2c17:
case 0x2c18:
case 0x2c19:
case 0x2c1a:
case 0x2c1b:
case 0x2c1c:
case 0x2c1d:
case 0x2c1e:
case 0x2c1f:
case 0x2c20:
case 0x2c21:
case 0x2c22:
case 0x2c23:
case 0x2c24:
case 0x2c25:
case 0x2c26:
case 0x2c27:
case 0x2c28:
case 0x2c29:
case 0x2c2a:
case 0x2c2b:
case 0x2c2c:
case 0x2c2d:
case 0x2c2e:
case 0x2c2f:
case 0x2c30:
case 0x2c31:
case 0x2c32:
case 0x2c33:
case 0x2c34:
case 0x2c35:
case 0x2c36:
case 0x2c37:
case 0x2c38:
case 0x2c39:
case 0x2c3a:
case 0x2c3b:
case 0x2c3c:
case 0x2c3d:
case 0x2c3e:
case 0x2c3f:
case 0x2c40:
case 0x2c41:
case 0x2c42:
case 0x2c43:
case 0x2c44:
case 0x2c45:
case 0x2c46:
case 0x2c47:
case 0x2c48:
case 0x2c49:
case 0x2c4a:
case 0x2c4b:
case 0x2c4c:
case 0x2c4d:
case 0x2c4e:
case 0x2c4f:
case 0x2c50:
case 0x2c51:
case 0x2c52:
case 0x2c53:
case 0x2c54:
case 0x2c55:
case 0x2c56:
case 0x2c57:
case 0x2c58:
case 0x2c59:
case 0x2c5a:
case 0x2c5b:
case 0x2c5c:
case 0x2c5d:
case 0x2c5e:
case 0x2c5f:
case 0x2c60:
case 0x2c61:
case 0x2c62:
case 0x2c63:
case 0x2c64:
case 0x2c65:
case 0x2c66:
case 0x2c67:
case 0x2c68:
case 0x2c69:
case 0x2c6a:
case 0x2c6b:
case 0x2c6c:
case 0x2c6d:
case 0x2c6e:
case 0x2c6f:
case 0x2c70:
case 0x2c71:
case 0x2c72:
case 0x2c73:
case 0x2c74:
case 0x2c75:
case 0x2c76:
case 0x2c77:
case 0x2c78:
case 0x2c79:
case 0x2c7a:
case 0x2c7b:
case 0x2c7c:
case 0x2c7d:
case 0x2c7e:
case 0x2c7f:
case 0x2c80:
case 0x2c81:
case 0x2c82:
case 0x2c83:
case 0x2c84:
case 0x2c85:
case 0x2c86:
case 0x2c87:
case 0x2c88:
case 0x2c89:
case 0x2c8a:
case 0x2c8b:
case 0x2c8c:
case 0x2c8d:
case 0x2c8e:
case 0x2c8f:
case 0x2c90:
case 0x2c91:
case 0x2c92:
case 0x2c93:
case 0x2c94:
case 0x2c95:
case 0x2c96:
case 0x2c97:
case 0x2c98:
case 0x2c99:
case 0x2c9a:
case 0x2c9b:
case 0x2c9c:
case 0x2c9d:
case 0x2c9e:
case 0x2c9f:
case 0x2ca0:
case 0x2ca1:
case 0x2ca2:
case 0x2ca3:
case 0x2ca4:
case 0x2ca5:
case 0x2ca6:
case 0x2ca7:
case 0x2ca8:
case 0x2ca9:
case 0x2caa:
case 0x2cab:
case 0x2cac:
case 0x2cad:
case 0x2cae:
case 0x2caf:
case 0x2cb0:
case 0x2cb1:
case 0x2cb2:
case 0x2cb3:
case 0x2cb4:
case 0x2cb5:
case 0x2cb6:
case 0x2cb7:
case 0x2cb8:
case 0x2cb9:
case 0x2cba:
case 0x2cbb:
case 0x2cbc:
case 0x2cbd:
case 0x2cbe:
case 0x2cbf:
case 0x2cc0:
case 0x2cc1:
case 0x2cc2:
case 0x2cc3:
case 0x2cc4:
case 0x2cc5:
case 0x2cc6:
case 0x2cc7:
case 0x2cc8:
case 0x2cc9:
case 0x2cca:
case 0x2ccb:
case 0x2ccc:
case 0x2ccd:
case 0x2cce:
case 0x2ccf:
case 0x2cd0:
case 0x2cd1:
case 0x2cd2:
case 0x2cd3:
case 0x2cd4:
case 0x2cd5:
case 0x2cd6:
case 0x2cd7:
case 0x2cd8:
case 0x2cd9:
case 0x2cda:
case 0x2cdb:
case 0x2cdc:
case 0x2cdd:
case 0x2cde:
case 0x2cdf:
case 0x2ce0:
case 0x2ce1:
case 0x2ce2:
case 0x2ce3:
case 0x2ce4:
case 0x2ce5:
case 0x2ce6:
case 0x2ce7:
case 0x2ce8:
case 0x2ce9:
case 0x2cea:
case 0x2ceb:
case 0x2cec:
case 0x2ced:
case 0x2cee:
case 0x2cef:
case 0x2cf0:
case 0x2cf1:
case 0x2cf2:
case 0x2cf3:
case 0x2cf4:
case 0x2cf5:
case 0x2cf6:
case 0x2cf7:
case 0x2cf8:
case 0x2cf9:
case 0x2cfa:
case 0x2cfb:
case 0x2cfc:
case 0x2cfd:
case 0x2cfe:
case 0x2cff:
case 0x2d00:
case 0x2d01:
case 0x2d02:
case 0x2d03:
case 0x2d04:
case 0x2d05:
case 0x2d06:
case 0x2d07:
case 0x2d08:
case 0x2d09:
case 0x2d0a:
case 0x2d0b:
case 0x2d0c:
case 0x2d0d:
case 0x2d0e:
case 0x2d0f:
case 0x2d10:
case 0x2d11:
case 0x2d12:
case 0x2d13:
case 0x2d14:
case 0x2d15:
case 0x2d16:
case 0x2d17:
case 0x2d18:
case 0x2d19:
case 0x2d1a:
case 0x2d1b:
case 0x2d1c:
case 0x2d1d:
case 0x2d1e:
case 0x2d1f:
case 0x2d20:
case 0x2d21:
case 0x2d22:
case 0x2d23:
case 0x2d24:
case 0x2d25:
case 0x2d26:
case 0x2d27:
case 0x2d28:
case 0x2d29:
case 0x2d2a:
case 0x2d2b:
case 0x2d2c:
case 0x2d2d:
case 0x2d2e:
case 0x2d2f:
case 0x2d30:
case 0x2d31:
case 0x2d32:
case 0x2d33:
case 0x2d34:
case 0x2d35:
case 0x2d36:
case 0x2d37:
case 0x2d38:
case 0x2d39:
case 0x2d3a:
case 0x2d3b:
case 0x2d3c:
case 0x2d3d:
case 0x2d3e:
case 0x2d3f:
case 0x2d40:
case 0x2d41:
case 0x2d42:
case 0x2d43:
case 0x2d44:
case 0x2d45:
case 0x2d46:
case 0x2d47:
case 0x2d48:
case 0x2d49:
case 0x2d4a:
case 0x2d4b:
case 0x2d4c:
case 0x2d4d:
case 0x2d4e:
case 0x2d4f:
case 0x2d50:
case 0x2d51:
case 0x2d52:
case 0x2d53:
case 0x2d54:
case 0x2d55:
case 0x2d56:
case 0x2d57:
case 0x2d58:
case 0x2d59:
case 0x2d5a:
case 0x2d5b:
case 0x2d5c:
case 0x2d5d:
case 0x2d5e:
case 0x2d5f:
case 0x2d60:
case 0x2d61:
case 0x2d62:
case 0x2d63:
case 0x2d64:
case 0x2d65:
case 0x2d66:
case 0x2d67:
case 0x2d68:
case 0x2d69:
case 0x2d6a:
case 0x2d6b:
case 0x2d6c:
case 0x2d6d:
case 0x2d6e:
case 0x2d6f:
case 0x2d70:
case 0x2d71:
case 0x2d72:
case 0x2d73:
case 0x2d74:
case 0x2d75:
case 0x2d76:
case 0x2d77:
case 0x2d78:
case 0x2d79:
case 0x2d7a:
case 0x2d7b:
case 0x2d7c:
case 0x2d7d:
case 0x2d7e:
case 0x2d7f:
case 0x2d80:
case 0x2d81:
case 0x2d82:
case 0x2d83:
case 0x2d84:
case 0x2d85:
case 0x2d86:
case 0x2d87:
case 0x2d88:
case 0x2d89:
case 0x2d8a:
case 0x2d8b:
case 0x2d8c:
case 0x2d8d:
case 0x2d8e:
case 0x2d8f:
case 0x2d90:
case 0x2d91:
case 0x2d92:
case 0x2d93:
case 0x2d94:
case 0x2d95:
case 0x2d96:
case 0x2d97:
case 0x2d98:
case 0x2d99:
case 0x2d9a:
case 0x2d9b:
case 0x2d9c:
case 0x2d9d:
case 0x2d9e:
case 0x2d9f:
case 0x2da0:
case 0x2da1:
case 0x2da2:
case 0x2da3:
case 0x2da4:
case 0x2da5:
case 0x2da6:
case 0x2da7:
case 0x2da8:
case 0x2da9:
case 0x2daa:
case 0x2dab:
case 0x2dac:
case 0x2dad:
case 0x2dae:
case 0x2daf:
case 0x2db0:
case 0x2db1:
case 0x2db2:
case 0x2db3:
case 0x2db4:
case 0x2db5:
case 0x2db6:
case 0x2db7:
case 0x2db8:
case 0x2db9:
case 0x2dba:
case 0x2dbb:
case 0x2dbc:
case 0x2dbd:
case 0x2dbe:
case 0x2dbf:
case 0x2dc0:
case 0x2dc1:
case 0x2dc2:
case 0x2dc3:
case 0x2dc4:
case 0x2dc5:
case 0x2dc6:
case 0x2dc7:
case 0x2dc8:
case 0x2dc9:
case 0x2dca:
case 0x2dcb:
case 0x2dcc:
case 0x2dcd:
case 0x2dce:
case 0x2dcf:
case 0x2dd0:
case 0x2dd1:
case 0x2dd2:
case 0x2dd3:
case 0x2dd4:
case 0x2dd5:
case 0x2dd6:
case 0x2dd7:
case 0x2dd8:
case 0x2dd9:
case 0x2dda:
case 0x2ddb:
case 0x2ddc:
case 0x2ddd:
case 0x2dde:
case 0x2ddf:
case 0x2de0:
case 0x2de1:
case 0x2de2:
case 0x2de3:
case 0x2de4:
case 0x2de5:
case 0x2de6:
case 0x2de7:
case 0x2de8:
case 0x2de9:
case 0x2dea:
case 0x2deb:
case 0x2dec:
case 0x2ded:
case 0x2dee:
case 0x2def:
case 0x2df0:
case 0x2df1:
case 0x2df2:
case 0x2df3:
case 0x2df4:
case 0x2df5:
case 0x2df6:
case 0x2df7:
case 0x2df8:
case 0x2df9:
case 0x2dfa:
case 0x2dfb:
case 0x2dfc:
case 0x2dfd:
case 0x2dfe:
case 0x2dff:
case 0x2e00:
case 0x2e01:
case 0x2e02:
case 0x2e03:
case 0x2e04:
case 0x2e05:
case 0x2e06:
case 0x2e07:
case 0x2e08:
case 0x2e09:
case 0x2e0a:
case 0x2e0b:
case 0x2e0c:
case 0x2e0d:
case 0x2e0e:
case 0x2e0f:
case 0x2e10:
case 0x2e11:
case 0x2e12:
case 0x2e13:
case 0x2e14:
case 0x2e15:
case 0x2e16:
case 0x2e17:
case 0x2e18:
case 0x2e19:
case 0x2e1a:
case 0x2e1b:
case 0x2e1c:
case 0x2e1d:
case 0x2e1e:
case 0x2e1f:
case 0x2e20:
case 0x2e21:
case 0x2e22:
case 0x2e23:
case 0x2e24:
case 0x2e25:
case 0x2e26:
case 0x2e27:
case 0x2e28:
case 0x2e29:
case 0x2e2a:
case 0x2e2b:
case 0x2e2c:
case 0x2e2d:
case 0x2e2e:
case 0x2e2f:
case 0x2e30:
case 0x2e31:
case 0x2e32:
case 0x2e33:
case 0x2e34:
case 0x2e35:
case 0x2e36:
case 0x2e37:
case 0x2e38:
case 0x2e39:
case 0x2e3a:
case 0x2e3b:
case 0x2e3c:
case 0x2e3d:
case 0x2e3e:
case 0x2e3f:
case 0x2e40:
case 0x2e41:
case 0x2e42:
case 0x2e43:
case 0x2e44:
case 0x2e45:
case 0x2e46:
case 0x2e47:
case 0x2e48:
case 0x2e49:
case 0x2e4a:
case 0x2e4b:
case 0x2e4c:
case 0x2e4d:
case 0x2e4e:
case 0x2e4f:
case 0x2e50:
case 0x2e51:
case 0x2e52:
case 0x2e53:
case 0x2e54:
case 0x2e55:
case 0x2e56:
case 0x2e57:
case 0x2e58:
case 0x2e59:
case 0x2e5a:
case 0x2e5b:
case 0x2e5c:
case 0x2e5d:
case 0x2e5e:
case 0x2e5f:
case 0x2e60:
case 0x2e61:
case 0x2e62:
case 0x2e63:
case 0x2e64:
case 0x2e65:
case 0x2e66:
case 0x2e67:
case 0x2e68:
case 0x2e69:
case 0x2e6a:
case 0x2e6b:
case 0x2e6c:
case 0x2e6d:
case 0x2e6e:
case 0x2e6f:
case 0x2e70:
case 0x2e71:
case 0x2e72:
case 0x2e73:
case 0x2e74:
case 0x2e75:
case 0x2e76:
case 0x2e77:
case 0x2e78:
case 0x2e79:
case 0x2e7a:
case 0x2e7b:
case 0x2e7c:
case 0x2e7d:
case 0x2e7e:
case 0x2e7f:
case 0x2e80:
case 0x2e81:
case 0x2e82:
case 0x2e83:
case 0x2e84:
case 0x2e85:
case 0x2e86:
case 0x2e87:
case 0x2e88:
case 0x2e89:
case 0x2e8a:
case 0x2e8b:
case 0x2e8c:
case 0x2e8d:
case 0x2e8e:
case 0x2e8f:
case 0x2e90:
case 0x2e91:
case 0x2e92:
case 0x2e93:
case 0x2e94:
case 0x2e95:
case 0x2e96:
case 0x2e97:
case 0x2e98:
case 0x2e99:
case 0x2e9a:
case 0x2e9b:
case 0x2e9c:
case 0x2e9d:
case 0x2e9e:
case 0x2e9f:
case 0x2ea0:
case 0x2ea1:
case 0x2ea2:
case 0x2ea3:
case 0x2ea4:
case 0x2ea5:
case 0x2ea6:
case 0x2ea7:
case 0x2ea8:
case 0x2ea9:
case 0x2eaa:
case 0x2eab:
case 0x2eac:
case 0x2ead:
case 0x2eae:
case 0x2eaf:
case 0x2eb0:
case 0x2eb1:
case 0x2eb2:
case 0x2eb3:
case 0x2eb4:
case 0x2eb5:
case 0x2eb6:
case 0x2eb7:
case 0x2eb8:
case 0x2eb9:
case 0x2eba:
case 0x2ebb:
case 0x2ebc:
case 0x2ebd:
case 0x2ebe:
case 0x2ebf:
case 0x2ec0:
case 0x2ec1:
case 0x2ec2:
case 0x2ec3:
case 0x2ec4:
case 0x2ec5:
case 0x2ec6:
case 0x2ec7:
case 0x2ec8:
case 0x2ec9:
case 0x2eca:
case 0x2ecb:
case 0x2ecc:
case 0x2ecd:
case 0x2ece:
case 0x2ecf:
case 0x2ed0:
case 0x2ed1:
case 0x2ed2:
case 0x2ed3:
case 0x2ed4:
case 0x2ed5:
case 0x2ed6:
case 0x2ed7:
case 0x2ed8:
case 0x2ed9:
case 0x2eda:
case 0x2edb:
case 0x2edc:
case 0x2edd:
case 0x2ede:
case 0x2edf:
case 0x2ee0:
case 0x2ee1:
case 0x2ee2:
case 0x2ee3:
case 0x2ee4:
case 0x2ee5:
case 0x2ee6:
case 0x2ee7:
case 0x2ee8:
case 0x2ee9:
case 0x2eea:
case 0x2eeb:
case 0x2eec:
case 0x2eed:
case 0x2eee:
case 0x2eef:
case 0x2ef0:
case 0x2ef1:
case 0x2ef2:
case 0x2ef3:
case 0x2ef4:
case 0x2ef5:
case 0x2ef6:
case 0x2ef7:
case 0x2ef8:
case 0x2ef9:
case 0x2efa:
case 0x2efb:
case 0x2efc:
case 0x2efd:
case 0x2efe:
case 0x2eff:
case 0x2f00:
case 0x2f01:
case 0x2f02:
case 0x2f03:
case 0x2f04:
case 0x2f05:
case 0x2f06:
case 0x2f07:
case 0x2f08:
case 0x2f09:
case 0x2f0a:
case 0x2f0b:
case 0x2f0c:
case 0x2f0d:
case 0x2f0e:
case 0x2f0f:
case 0x2f10:
case 0x2f11:
case 0x2f12:
case 0x2f13:
case 0x2f14:
case 0x2f15:
case 0x2f16:
case 0x2f17:
case 0x2f18:
case 0x2f19:
case 0x2f1a:
case 0x2f1b:
case 0x2f1c:
case 0x2f1d:
case 0x2f1e:
case 0x2f1f:
case 0x2f20:
case 0x2f21:
case 0x2f22:
case 0x2f23:
case 0x2f24:
case 0x2f25:
case 0x2f26:
case 0x2f27:
case 0x2f28:
case 0x2f29:
case 0x2f2a:
case 0x2f2b:
case 0x2f2c:
case 0x2f2d:
case 0x2f2e:
case 0x2f2f:
case 0x2f30:
case 0x2f31:
case 0x2f32:
case 0x2f33:
case 0x2f34:
case 0x2f35:
case 0x2f36:
case 0x2f37:
case 0x2f38:
case 0x2f39:
case 0x2f3a:
case 0x2f3b:
case 0x2f3c:
case 0x2f3d:
case 0x2f3e:
case 0x2f3f:
case 0x2f40:
case 0x2f41:
case 0x2f42:
case 0x2f43:
case 0x2f44:
case 0x2f45:
case 0x2f46:
case 0x2f47:
case 0x2f48:
case 0x2f49:
case 0x2f4a:
case 0x2f4b:
case 0x2f4c:
case 0x2f4d:
case 0x2f4e:
case 0x2f4f:
case 0x2f50:
case 0x2f51:
case 0x2f52:
case 0x2f53:
case 0x2f54:
case 0x2f55:
case 0x2f56:
case 0x2f57:
case 0x2f58:
case 0x2f59:
case 0x2f5a:
case 0x2f5b:
case 0x2f5c:
case 0x2f5d:
case 0x2f5e:
case 0x2f5f:
case 0x2f60:
case 0x2f61:
case 0x2f62:
case 0x2f63:
case 0x2f64:
case 0x2f65:
case 0x2f66:
case 0x2f67:
case 0x2f68:
case 0x2f69:
case 0x2f6a:
case 0x2f6b:
case 0x2f6c:
case 0x2f6d:
case 0x2f6e:
case 0x2f6f:
case 0x2f70:
case 0x2f71:
case 0x2f72:
case 0x2f73:
case 0x2f74:
case 0x2f75:
case 0x2f76:
case 0x2f77:
case 0x2f78:
case 0x2f79:
case 0x2f7a:
case 0x2f7b:
case 0x2f7c:
case 0x2f7d:
case 0x2f7e:
case 0x2f7f:
case 0x2f80:
case 0x2f81:
case 0x2f82:
case 0x2f83:
case 0x2f84:
case 0x2f85:
case 0x2f86:
case 0x2f87:
case 0x2f88:
case 0x2f89:
case 0x2f8a:
case 0x2f8b:
case 0x2f8c:
case 0x2f8d:
case 0x2f8e:
case 0x2f8f:
case 0x2f90:
case 0x2f91:
case 0x2f92:
case 0x2f93:
case 0x2f94:
case 0x2f95:
case 0x2f96:
case 0x2f97:
case 0x2f98:
case 0x2f99:
case 0x2f9a:
case 0x2f9b:
case 0x2f9c:
case 0x2f9d:
case 0x2f9e:
case 0x2f9f:
case 0x2fa0:
case 0x2fa1:
case 0x2fa2:
case 0x2fa3:
case 0x2fa4:
case 0x2fa5:
case 0x2fa6:
case 0x2fa7:
case 0x2fa8:
case 0x2fa9:
case 0x2faa:
case 0x2fab:
case 0x2fac:
case 0x2fad:
case 0x2fae:
case 0x2faf:
case 0x2fb0:
case 0x2fb1:
case 0x2fb2:
case 0x2fb3:
case 0x2fb4:
case 0x2fb5:
case 0x2fb6:
case 0x2fb7:
case 0x2fb8:
case 0x2fb9:
case 0x2fba:
case 0x2fbb:
case 0x2fbc:
case 0x2fbd:
case 0x2fbe:
case 0x2fbf:
case 0x2fc0:
case 0x2fc1:
case 0x2fc2:
case 0x2fc3:
case 0x2fc4:
case 0x2fc5:
case 0x2fc6:
case 0x2fc7:
case 0x2fc8:
case 0x2fc9:
case 0x2fca:
case 0x2fcb:
case 0x2fcc:
case 0x2fcd:
case 0x2fce:
case 0x2fcf:
case 0x2fd0:
case 0x2fd1:
case 0x2fd2:
case 0x2fd3:
case 0x2fd4:
case 0x2fd5:
case 0x2fd6:
case 0x2fd7:
case 0x2fd8:
case 0x2fd9:
case 0x2fda:
case 0x2fdb:
case 0x2fdc:
case 0x2fdd:
case 0x2fde:
case 0x2fdf:
case 0x2fe0:
case 0x2fe1:
case 0x2fe2:
case 0x2fe3:
case 0x2fe4:
case 0x2fe5:
case 0x2fe6:
case 0x2fe7:
case 0x2fe8:
case 0x2fe9:
case 0x2fea:
case 0x2feb:
case 0x2fec:
case 0x2fed:
case 0x2fee:
case 0x2fef:
case 0x2ff0:
case 0x2ff1:
case 0x2ff2:
case 0x2ff3:
case 0x2ff4:
case 0x2ff5:
case 0x2ff6:
case 0x2ff7:
case 0x2ff8:
case 0x2ff9:
case 0x2ffa:
case 0x2ffb:
case 0x2ffc:
case 0x2ffd:
case 0x2ffe:
case 0x2fff:
	{
	 LINEA();
	}
	break;

case 0x3000:
case 0x3001:
case 0x3002:
case 0x3003:
case 0x3004:
case 0x3005:
case 0x3006:
case 0x3007:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3010:
case 0x3011:
case 0x3012:
case 0x3013:
case 0x3014:
case 0x3015:
case 0x3016:
case 0x3017:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3018:
case 0x3019:
case 0x301a:
case 0x301b:
case 0x301c:
case 0x301d:
case 0x301e:
case 0x301f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3020:
case 0x3021:
case 0x3022:
case 0x3023:
case 0x3024:
case 0x3025:
case 0x3026:
case 0x3027:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3028:
case 0x3029:
case 0x302a:
case 0x302b:
case 0x302c:
case 0x302d:
case 0x302e:
case 0x302f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3030:
case 0x3031:
case 0x3032:
case 0x3033:
case 0x3034:
case 0x3035:
case 0x3036:
case 0x3037:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3038:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3039:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x303a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x303b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x303c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3040:
case 0x3041:
case 0x3042:
case 0x3043:
case 0x3044:
case 0x3045:
case 0x3046:
case 0x3047:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3048:
case 0x3049:
case 0x304a:
case 0x304b:
case 0x304c:
case 0x304d:
case 0x304e:
case 0x304f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3050:
case 0x3051:
case 0x3052:
case 0x3053:
case 0x3054:
case 0x3055:
case 0x3056:
case 0x3057:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3058:
case 0x3059:
case 0x305a:
case 0x305b:
case 0x305c:
case 0x305d:
case 0x305e:
case 0x305f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3060:
case 0x3061:
case 0x3062:
case 0x3063:
case 0x3064:
case 0x3065:
case 0x3066:
case 0x3067:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3068:
case 0x3069:
case 0x306a:
case 0x306b:
case 0x306c:
case 0x306d:
case 0x306e:
case 0x306f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3070:
case 0x3071:
case 0x3072:
case 0x3073:
case 0x3074:
case 0x3075:
case 0x3076:
case 0x3077:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3078:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3079:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x307a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x307b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x307c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3080:
case 0x3081:
case 0x3082:
case 0x3083:
case 0x3084:
case 0x3085:
case 0x3086:
case 0x3087:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3088:
case 0x3089:
case 0x308a:
case 0x308b:
case 0x308c:
case 0x308d:
case 0x308e:
case 0x308f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3090:
case 0x3091:
case 0x3092:
case 0x3093:
case 0x3094:
case 0x3095:
case 0x3096:
case 0x3097:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3098:
case 0x3099:
case 0x309a:
case 0x309b:
case 0x309c:
case 0x309d:
case 0x309e:
case 0x309f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30a0:
case 0x30a1:
case 0x30a2:
case 0x30a3:
case 0x30a4:
case 0x30a5:
case 0x30a6:
case 0x30a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30a8:
case 0x30a9:
case 0x30aa:
case 0x30ab:
case 0x30ac:
case 0x30ad:
case 0x30ae:
case 0x30af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30b0:
case 0x30b1:
case 0x30b2:
case 0x30b3:
case 0x30b4:
case 0x30b5:
case 0x30b6:
case 0x30b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30c0:
case 0x30c1:
case 0x30c2:
case 0x30c3:
case 0x30c4:
case 0x30c5:
case 0x30c6:
case 0x30c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30c8:
case 0x30c9:
case 0x30ca:
case 0x30cb:
case 0x30cc:
case 0x30cd:
case 0x30ce:
case 0x30cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30d0:
case 0x30d1:
case 0x30d2:
case 0x30d3:
case 0x30d4:
case 0x30d5:
case 0x30d6:
case 0x30d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30d8:
case 0x30d9:
case 0x30da:
case 0x30db:
case 0x30dc:
case 0x30dd:
case 0x30de:
case 0x30df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30e0:
case 0x30e1:
case 0x30e2:
case 0x30e3:
case 0x30e4:
case 0x30e5:
case 0x30e6:
case 0x30e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30e8:
case 0x30e9:
case 0x30ea:
case 0x30eb:
case 0x30ec:
case 0x30ed:
case 0x30ee:
case 0x30ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30f0:
case 0x30f1:
case 0x30f2:
case 0x30f3:
case 0x30f4:
case 0x30f5:
case 0x30f6:
case 0x30f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x30fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3100:
case 0x3101:
case 0x3102:
case 0x3103:
case 0x3104:
case 0x3105:
case 0x3106:
case 0x3107:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3108:
case 0x3109:
case 0x310a:
case 0x310b:
case 0x310c:
case 0x310d:
case 0x310e:
case 0x310f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3110:
case 0x3111:
case 0x3112:
case 0x3113:
case 0x3114:
case 0x3115:
case 0x3116:
case 0x3117:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3118:
case 0x3119:
case 0x311a:
case 0x311b:
case 0x311c:
case 0x311d:
case 0x311e:
case 0x311f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3120:
case 0x3121:
case 0x3122:
case 0x3123:
case 0x3124:
case 0x3125:
case 0x3126:
case 0x3127:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3128:
case 0x3129:
case 0x312a:
case 0x312b:
case 0x312c:
case 0x312d:
case 0x312e:
case 0x312f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3130:
case 0x3131:
case 0x3132:
case 0x3133:
case 0x3134:
case 0x3135:
case 0x3136:
case 0x3137:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3138:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3139:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3140:
case 0x3141:
case 0x3142:
case 0x3143:
case 0x3144:
case 0x3145:
case 0x3146:
case 0x3147:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3148:
case 0x3149:
case 0x314a:
case 0x314b:
case 0x314c:
case 0x314d:
case 0x314e:
case 0x314f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3150:
case 0x3151:
case 0x3152:
case 0x3153:
case 0x3154:
case 0x3155:
case 0x3156:
case 0x3157:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3158:
case 0x3159:
case 0x315a:
case 0x315b:
case 0x315c:
case 0x315d:
case 0x315e:
case 0x315f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3160:
case 0x3161:
case 0x3162:
case 0x3163:
case 0x3164:
case 0x3165:
case 0x3166:
case 0x3167:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3168:
case 0x3169:
case 0x316a:
case 0x316b:
case 0x316c:
case 0x316d:
case 0x316e:
case 0x316f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3170:
case 0x3171:
case 0x3172:
case 0x3173:
case 0x3174:
case 0x3175:
case 0x3176:
case 0x3177:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3178:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3179:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3180:
case 0x3181:
case 0x3182:
case 0x3183:
case 0x3184:
case 0x3185:
case 0x3186:
case 0x3187:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3188:
case 0x3189:
case 0x318a:
case 0x318b:
case 0x318c:
case 0x318d:
case 0x318e:
case 0x318f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3190:
case 0x3191:
case 0x3192:
case 0x3193:
case 0x3194:
case 0x3195:
case 0x3196:
case 0x3197:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3198:
case 0x3199:
case 0x319a:
case 0x319b:
case 0x319c:
case 0x319d:
case 0x319e:
case 0x319f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x31a0:
case 0x31a1:
case 0x31a2:
case 0x31a3:
case 0x31a4:
case 0x31a5:
case 0x31a6:
case 0x31a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x31a8:
case 0x31a9:
case 0x31aa:
case 0x31ab:
case 0x31ac:
case 0x31ad:
case 0x31ae:
case 0x31af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x31b0:
case 0x31b1:
case 0x31b2:
case 0x31b3:
case 0x31b4:
case 0x31b5:
case 0x31b6:
case 0x31b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x31b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x31b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x31c0:
case 0x31c1:
case 0x31c2:
case 0x31c3:
case 0x31c4:
case 0x31c5:
case 0x31c6:
case 0x31c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31c8:
case 0x31c9:
case 0x31ca:
case 0x31cb:
case 0x31cc:
case 0x31cd:
case 0x31ce:
case 0x31cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31d0:
case 0x31d1:
case 0x31d2:
case 0x31d3:
case 0x31d4:
case 0x31d5:
case 0x31d6:
case 0x31d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31d8:
case 0x31d9:
case 0x31da:
case 0x31db:
case 0x31dc:
case 0x31dd:
case 0x31de:
case 0x31df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31e0:
case 0x31e1:
case 0x31e2:
case 0x31e3:
case 0x31e4:
case 0x31e5:
case 0x31e6:
case 0x31e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31e8:
case 0x31e9:
case 0x31ea:
case 0x31eb:
case 0x31ec:
case 0x31ed:
case 0x31ee:
case 0x31ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31f0:
case 0x31f1:
case 0x31f2:
case 0x31f3:
case 0x31f4:
case 0x31f5:
case 0x31f6:
case 0x31f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x31fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3200:
case 0x3201:
case 0x3202:
case 0x3203:
case 0x3204:
case 0x3205:
case 0x3206:
case 0x3207:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3210:
case 0x3211:
case 0x3212:
case 0x3213:
case 0x3214:
case 0x3215:
case 0x3216:
case 0x3217:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3218:
case 0x3219:
case 0x321a:
case 0x321b:
case 0x321c:
case 0x321d:
case 0x321e:
case 0x321f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3220:
case 0x3221:
case 0x3222:
case 0x3223:
case 0x3224:
case 0x3225:
case 0x3226:
case 0x3227:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3228:
case 0x3229:
case 0x322a:
case 0x322b:
case 0x322c:
case 0x322d:
case 0x322e:
case 0x322f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3230:
case 0x3231:
case 0x3232:
case 0x3233:
case 0x3234:
case 0x3235:
case 0x3236:
case 0x3237:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3238:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3239:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x323a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x323b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x323c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3240:
case 0x3241:
case 0x3242:
case 0x3243:
case 0x3244:
case 0x3245:
case 0x3246:
case 0x3247:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3248:
case 0x3249:
case 0x324a:
case 0x324b:
case 0x324c:
case 0x324d:
case 0x324e:
case 0x324f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3250:
case 0x3251:
case 0x3252:
case 0x3253:
case 0x3254:
case 0x3255:
case 0x3256:
case 0x3257:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3258:
case 0x3259:
case 0x325a:
case 0x325b:
case 0x325c:
case 0x325d:
case 0x325e:
case 0x325f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3260:
case 0x3261:
case 0x3262:
case 0x3263:
case 0x3264:
case 0x3265:
case 0x3266:
case 0x3267:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3268:
case 0x3269:
case 0x326a:
case 0x326b:
case 0x326c:
case 0x326d:
case 0x326e:
case 0x326f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3270:
case 0x3271:
case 0x3272:
case 0x3273:
case 0x3274:
case 0x3275:
case 0x3276:
case 0x3277:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3278:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3279:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x327a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x327b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x327c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3280:
case 0x3281:
case 0x3282:
case 0x3283:
case 0x3284:
case 0x3285:
case 0x3286:
case 0x3287:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3288:
case 0x3289:
case 0x328a:
case 0x328b:
case 0x328c:
case 0x328d:
case 0x328e:
case 0x328f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3290:
case 0x3291:
case 0x3292:
case 0x3293:
case 0x3294:
case 0x3295:
case 0x3296:
case 0x3297:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3298:
case 0x3299:
case 0x329a:
case 0x329b:
case 0x329c:
case 0x329d:
case 0x329e:
case 0x329f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32a0:
case 0x32a1:
case 0x32a2:
case 0x32a3:
case 0x32a4:
case 0x32a5:
case 0x32a6:
case 0x32a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32a8:
case 0x32a9:
case 0x32aa:
case 0x32ab:
case 0x32ac:
case 0x32ad:
case 0x32ae:
case 0x32af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32b0:
case 0x32b1:
case 0x32b2:
case 0x32b3:
case 0x32b4:
case 0x32b5:
case 0x32b6:
case 0x32b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32c0:
case 0x32c1:
case 0x32c2:
case 0x32c3:
case 0x32c4:
case 0x32c5:
case 0x32c6:
case 0x32c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32c8:
case 0x32c9:
case 0x32ca:
case 0x32cb:
case 0x32cc:
case 0x32cd:
case 0x32ce:
case 0x32cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32d0:
case 0x32d1:
case 0x32d2:
case 0x32d3:
case 0x32d4:
case 0x32d5:
case 0x32d6:
case 0x32d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32d8:
case 0x32d9:
case 0x32da:
case 0x32db:
case 0x32dc:
case 0x32dd:
case 0x32de:
case 0x32df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32e0:
case 0x32e1:
case 0x32e2:
case 0x32e3:
case 0x32e4:
case 0x32e5:
case 0x32e6:
case 0x32e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32e8:
case 0x32e9:
case 0x32ea:
case 0x32eb:
case 0x32ec:
case 0x32ed:
case 0x32ee:
case 0x32ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32f0:
case 0x32f1:
case 0x32f2:
case 0x32f3:
case 0x32f4:
case 0x32f5:
case 0x32f6:
case 0x32f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x32fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3300:
case 0x3301:
case 0x3302:
case 0x3303:
case 0x3304:
case 0x3305:
case 0x3306:
case 0x3307:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3308:
case 0x3309:
case 0x330a:
case 0x330b:
case 0x330c:
case 0x330d:
case 0x330e:
case 0x330f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3310:
case 0x3311:
case 0x3312:
case 0x3313:
case 0x3314:
case 0x3315:
case 0x3316:
case 0x3317:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3318:
case 0x3319:
case 0x331a:
case 0x331b:
case 0x331c:
case 0x331d:
case 0x331e:
case 0x331f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3320:
case 0x3321:
case 0x3322:
case 0x3323:
case 0x3324:
case 0x3325:
case 0x3326:
case 0x3327:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3328:
case 0x3329:
case 0x332a:
case 0x332b:
case 0x332c:
case 0x332d:
case 0x332e:
case 0x332f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3330:
case 0x3331:
case 0x3332:
case 0x3333:
case 0x3334:
case 0x3335:
case 0x3336:
case 0x3337:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3338:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3339:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3340:
case 0x3341:
case 0x3342:
case 0x3343:
case 0x3344:
case 0x3345:
case 0x3346:
case 0x3347:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3348:
case 0x3349:
case 0x334a:
case 0x334b:
case 0x334c:
case 0x334d:
case 0x334e:
case 0x334f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3350:
case 0x3351:
case 0x3352:
case 0x3353:
case 0x3354:
case 0x3355:
case 0x3356:
case 0x3357:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3358:
case 0x3359:
case 0x335a:
case 0x335b:
case 0x335c:
case 0x335d:
case 0x335e:
case 0x335f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3360:
case 0x3361:
case 0x3362:
case 0x3363:
case 0x3364:
case 0x3365:
case 0x3366:
case 0x3367:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3368:
case 0x3369:
case 0x336a:
case 0x336b:
case 0x336c:
case 0x336d:
case 0x336e:
case 0x336f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3370:
case 0x3371:
case 0x3372:
case 0x3373:
case 0x3374:
case 0x3375:
case 0x3376:
case 0x3377:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3378:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3379:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3380:
case 0x3381:
case 0x3382:
case 0x3383:
case 0x3384:
case 0x3385:
case 0x3386:
case 0x3387:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3388:
case 0x3389:
case 0x338a:
case 0x338b:
case 0x338c:
case 0x338d:
case 0x338e:
case 0x338f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3390:
case 0x3391:
case 0x3392:
case 0x3393:
case 0x3394:
case 0x3395:
case 0x3396:
case 0x3397:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3398:
case 0x3399:
case 0x339a:
case 0x339b:
case 0x339c:
case 0x339d:
case 0x339e:
case 0x339f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x33a0:
case 0x33a1:
case 0x33a2:
case 0x33a3:
case 0x33a4:
case 0x33a5:
case 0x33a6:
case 0x33a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x33a8:
case 0x33a9:
case 0x33aa:
case 0x33ab:
case 0x33ac:
case 0x33ad:
case 0x33ae:
case 0x33af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x33b0:
case 0x33b1:
case 0x33b2:
case 0x33b3:
case 0x33b4:
case 0x33b5:
case 0x33b6:
case 0x33b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x33b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x33b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x33c0:
case 0x33c1:
case 0x33c2:
case 0x33c3:
case 0x33c4:
case 0x33c5:
case 0x33c6:
case 0x33c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33c8:
case 0x33c9:
case 0x33ca:
case 0x33cb:
case 0x33cc:
case 0x33cd:
case 0x33ce:
case 0x33cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33d0:
case 0x33d1:
case 0x33d2:
case 0x33d3:
case 0x33d4:
case 0x33d5:
case 0x33d6:
case 0x33d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33d8:
case 0x33d9:
case 0x33da:
case 0x33db:
case 0x33dc:
case 0x33dd:
case 0x33de:
case 0x33df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33e0:
case 0x33e1:
case 0x33e2:
case 0x33e3:
case 0x33e4:
case 0x33e5:
case 0x33e6:
case 0x33e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33e8:
case 0x33e9:
case 0x33ea:
case 0x33eb:
case 0x33ec:
case 0x33ed:
case 0x33ee:
case 0x33ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33f0:
case 0x33f1:
case 0x33f2:
case 0x33f3:
case 0x33f4:
case 0x33f5:
case 0x33f6:
case 0x33f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x33fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3400:
case 0x3401:
case 0x3402:
case 0x3403:
case 0x3404:
case 0x3405:
case 0x3406:
case 0x3407:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3410:
case 0x3411:
case 0x3412:
case 0x3413:
case 0x3414:
case 0x3415:
case 0x3416:
case 0x3417:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3418:
case 0x3419:
case 0x341a:
case 0x341b:
case 0x341c:
case 0x341d:
case 0x341e:
case 0x341f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3420:
case 0x3421:
case 0x3422:
case 0x3423:
case 0x3424:
case 0x3425:
case 0x3426:
case 0x3427:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3428:
case 0x3429:
case 0x342a:
case 0x342b:
case 0x342c:
case 0x342d:
case 0x342e:
case 0x342f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3430:
case 0x3431:
case 0x3432:
case 0x3433:
case 0x3434:
case 0x3435:
case 0x3436:
case 0x3437:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3438:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3439:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x343a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x343b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x343c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3440:
case 0x3441:
case 0x3442:
case 0x3443:
case 0x3444:
case 0x3445:
case 0x3446:
case 0x3447:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3448:
case 0x3449:
case 0x344a:
case 0x344b:
case 0x344c:
case 0x344d:
case 0x344e:
case 0x344f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3450:
case 0x3451:
case 0x3452:
case 0x3453:
case 0x3454:
case 0x3455:
case 0x3456:
case 0x3457:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3458:
case 0x3459:
case 0x345a:
case 0x345b:
case 0x345c:
case 0x345d:
case 0x345e:
case 0x345f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3460:
case 0x3461:
case 0x3462:
case 0x3463:
case 0x3464:
case 0x3465:
case 0x3466:
case 0x3467:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3468:
case 0x3469:
case 0x346a:
case 0x346b:
case 0x346c:
case 0x346d:
case 0x346e:
case 0x346f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3470:
case 0x3471:
case 0x3472:
case 0x3473:
case 0x3474:
case 0x3475:
case 0x3476:
case 0x3477:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3478:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3479:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x347a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x347b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x347c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3480:
case 0x3481:
case 0x3482:
case 0x3483:
case 0x3484:
case 0x3485:
case 0x3486:
case 0x3487:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3488:
case 0x3489:
case 0x348a:
case 0x348b:
case 0x348c:
case 0x348d:
case 0x348e:
case 0x348f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3490:
case 0x3491:
case 0x3492:
case 0x3493:
case 0x3494:
case 0x3495:
case 0x3496:
case 0x3497:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3498:
case 0x3499:
case 0x349a:
case 0x349b:
case 0x349c:
case 0x349d:
case 0x349e:
case 0x349f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34a0:
case 0x34a1:
case 0x34a2:
case 0x34a3:
case 0x34a4:
case 0x34a5:
case 0x34a6:
case 0x34a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34a8:
case 0x34a9:
case 0x34aa:
case 0x34ab:
case 0x34ac:
case 0x34ad:
case 0x34ae:
case 0x34af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34b0:
case 0x34b1:
case 0x34b2:
case 0x34b3:
case 0x34b4:
case 0x34b5:
case 0x34b6:
case 0x34b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34c0:
case 0x34c1:
case 0x34c2:
case 0x34c3:
case 0x34c4:
case 0x34c5:
case 0x34c6:
case 0x34c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34c8:
case 0x34c9:
case 0x34ca:
case 0x34cb:
case 0x34cc:
case 0x34cd:
case 0x34ce:
case 0x34cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34d0:
case 0x34d1:
case 0x34d2:
case 0x34d3:
case 0x34d4:
case 0x34d5:
case 0x34d6:
case 0x34d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34d8:
case 0x34d9:
case 0x34da:
case 0x34db:
case 0x34dc:
case 0x34dd:
case 0x34de:
case 0x34df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34e0:
case 0x34e1:
case 0x34e2:
case 0x34e3:
case 0x34e4:
case 0x34e5:
case 0x34e6:
case 0x34e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34e8:
case 0x34e9:
case 0x34ea:
case 0x34eb:
case 0x34ec:
case 0x34ed:
case 0x34ee:
case 0x34ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34f0:
case 0x34f1:
case 0x34f2:
case 0x34f3:
case 0x34f4:
case 0x34f5:
case 0x34f6:
case 0x34f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x34fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3500:
case 0x3501:
case 0x3502:
case 0x3503:
case 0x3504:
case 0x3505:
case 0x3506:
case 0x3507:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3508:
case 0x3509:
case 0x350a:
case 0x350b:
case 0x350c:
case 0x350d:
case 0x350e:
case 0x350f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3510:
case 0x3511:
case 0x3512:
case 0x3513:
case 0x3514:
case 0x3515:
case 0x3516:
case 0x3517:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3518:
case 0x3519:
case 0x351a:
case 0x351b:
case 0x351c:
case 0x351d:
case 0x351e:
case 0x351f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3520:
case 0x3521:
case 0x3522:
case 0x3523:
case 0x3524:
case 0x3525:
case 0x3526:
case 0x3527:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3528:
case 0x3529:
case 0x352a:
case 0x352b:
case 0x352c:
case 0x352d:
case 0x352e:
case 0x352f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3530:
case 0x3531:
case 0x3532:
case 0x3533:
case 0x3534:
case 0x3535:
case 0x3536:
case 0x3537:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3538:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3539:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3540:
case 0x3541:
case 0x3542:
case 0x3543:
case 0x3544:
case 0x3545:
case 0x3546:
case 0x3547:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3548:
case 0x3549:
case 0x354a:
case 0x354b:
case 0x354c:
case 0x354d:
case 0x354e:
case 0x354f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3550:
case 0x3551:
case 0x3552:
case 0x3553:
case 0x3554:
case 0x3555:
case 0x3556:
case 0x3557:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3558:
case 0x3559:
case 0x355a:
case 0x355b:
case 0x355c:
case 0x355d:
case 0x355e:
case 0x355f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3560:
case 0x3561:
case 0x3562:
case 0x3563:
case 0x3564:
case 0x3565:
case 0x3566:
case 0x3567:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3568:
case 0x3569:
case 0x356a:
case 0x356b:
case 0x356c:
case 0x356d:
case 0x356e:
case 0x356f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3570:
case 0x3571:
case 0x3572:
case 0x3573:
case 0x3574:
case 0x3575:
case 0x3576:
case 0x3577:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3578:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3579:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3580:
case 0x3581:
case 0x3582:
case 0x3583:
case 0x3584:
case 0x3585:
case 0x3586:
case 0x3587:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3588:
case 0x3589:
case 0x358a:
case 0x358b:
case 0x358c:
case 0x358d:
case 0x358e:
case 0x358f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3590:
case 0x3591:
case 0x3592:
case 0x3593:
case 0x3594:
case 0x3595:
case 0x3596:
case 0x3597:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3598:
case 0x3599:
case 0x359a:
case 0x359b:
case 0x359c:
case 0x359d:
case 0x359e:
case 0x359f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x35a0:
case 0x35a1:
case 0x35a2:
case 0x35a3:
case 0x35a4:
case 0x35a5:
case 0x35a6:
case 0x35a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x35a8:
case 0x35a9:
case 0x35aa:
case 0x35ab:
case 0x35ac:
case 0x35ad:
case 0x35ae:
case 0x35af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x35b0:
case 0x35b1:
case 0x35b2:
case 0x35b3:
case 0x35b4:
case 0x35b5:
case 0x35b6:
case 0x35b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x35b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x35b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x35c0:
case 0x35c1:
case 0x35c2:
case 0x35c3:
case 0x35c4:
case 0x35c5:
case 0x35c6:
case 0x35c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35c8:
case 0x35c9:
case 0x35ca:
case 0x35cb:
case 0x35cc:
case 0x35cd:
case 0x35ce:
case 0x35cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35d0:
case 0x35d1:
case 0x35d2:
case 0x35d3:
case 0x35d4:
case 0x35d5:
case 0x35d6:
case 0x35d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35d8:
case 0x35d9:
case 0x35da:
case 0x35db:
case 0x35dc:
case 0x35dd:
case 0x35de:
case 0x35df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35e0:
case 0x35e1:
case 0x35e2:
case 0x35e3:
case 0x35e4:
case 0x35e5:
case 0x35e6:
case 0x35e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35e8:
case 0x35e9:
case 0x35ea:
case 0x35eb:
case 0x35ec:
case 0x35ed:
case 0x35ee:
case 0x35ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35f0:
case 0x35f1:
case 0x35f2:
case 0x35f3:
case 0x35f4:
case 0x35f5:
case 0x35f6:
case 0x35f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x35fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3600:
case 0x3601:
case 0x3602:
case 0x3603:
case 0x3604:
case 0x3605:
case 0x3606:
case 0x3607:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3610:
case 0x3611:
case 0x3612:
case 0x3613:
case 0x3614:
case 0x3615:
case 0x3616:
case 0x3617:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3618:
case 0x3619:
case 0x361a:
case 0x361b:
case 0x361c:
case 0x361d:
case 0x361e:
case 0x361f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3620:
case 0x3621:
case 0x3622:
case 0x3623:
case 0x3624:
case 0x3625:
case 0x3626:
case 0x3627:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3628:
case 0x3629:
case 0x362a:
case 0x362b:
case 0x362c:
case 0x362d:
case 0x362e:
case 0x362f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3630:
case 0x3631:
case 0x3632:
case 0x3633:
case 0x3634:
case 0x3635:
case 0x3636:
case 0x3637:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3638:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3639:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x363a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x363b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x363c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3640:
case 0x3641:
case 0x3642:
case 0x3643:
case 0x3644:
case 0x3645:
case 0x3646:
case 0x3647:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3648:
case 0x3649:
case 0x364a:
case 0x364b:
case 0x364c:
case 0x364d:
case 0x364e:
case 0x364f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3650:
case 0x3651:
case 0x3652:
case 0x3653:
case 0x3654:
case 0x3655:
case 0x3656:
case 0x3657:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3658:
case 0x3659:
case 0x365a:
case 0x365b:
case 0x365c:
case 0x365d:
case 0x365e:
case 0x365f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3660:
case 0x3661:
case 0x3662:
case 0x3663:
case 0x3664:
case 0x3665:
case 0x3666:
case 0x3667:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3668:
case 0x3669:
case 0x366a:
case 0x366b:
case 0x366c:
case 0x366d:
case 0x366e:
case 0x366f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3670:
case 0x3671:
case 0x3672:
case 0x3673:
case 0x3674:
case 0x3675:
case 0x3676:
case 0x3677:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3678:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3679:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x367a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x367b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x367c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3680:
case 0x3681:
case 0x3682:
case 0x3683:
case 0x3684:
case 0x3685:
case 0x3686:
case 0x3687:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3688:
case 0x3689:
case 0x368a:
case 0x368b:
case 0x368c:
case 0x368d:
case 0x368e:
case 0x368f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3690:
case 0x3691:
case 0x3692:
case 0x3693:
case 0x3694:
case 0x3695:
case 0x3696:
case 0x3697:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3698:
case 0x3699:
case 0x369a:
case 0x369b:
case 0x369c:
case 0x369d:
case 0x369e:
case 0x369f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36a0:
case 0x36a1:
case 0x36a2:
case 0x36a3:
case 0x36a4:
case 0x36a5:
case 0x36a6:
case 0x36a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36a8:
case 0x36a9:
case 0x36aa:
case 0x36ab:
case 0x36ac:
case 0x36ad:
case 0x36ae:
case 0x36af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36b0:
case 0x36b1:
case 0x36b2:
case 0x36b3:
case 0x36b4:
case 0x36b5:
case 0x36b6:
case 0x36b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36c0:
case 0x36c1:
case 0x36c2:
case 0x36c3:
case 0x36c4:
case 0x36c5:
case 0x36c6:
case 0x36c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36c8:
case 0x36c9:
case 0x36ca:
case 0x36cb:
case 0x36cc:
case 0x36cd:
case 0x36ce:
case 0x36cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36d0:
case 0x36d1:
case 0x36d2:
case 0x36d3:
case 0x36d4:
case 0x36d5:
case 0x36d6:
case 0x36d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36d8:
case 0x36d9:
case 0x36da:
case 0x36db:
case 0x36dc:
case 0x36dd:
case 0x36de:
case 0x36df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36e0:
case 0x36e1:
case 0x36e2:
case 0x36e3:
case 0x36e4:
case 0x36e5:
case 0x36e6:
case 0x36e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36e8:
case 0x36e9:
case 0x36ea:
case 0x36eb:
case 0x36ec:
case 0x36ed:
case 0x36ee:
case 0x36ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36f0:
case 0x36f1:
case 0x36f2:
case 0x36f3:
case 0x36f4:
case 0x36f5:
case 0x36f6:
case 0x36f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x36fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3700:
case 0x3701:
case 0x3702:
case 0x3703:
case 0x3704:
case 0x3705:
case 0x3706:
case 0x3707:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3708:
case 0x3709:
case 0x370a:
case 0x370b:
case 0x370c:
case 0x370d:
case 0x370e:
case 0x370f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3710:
case 0x3711:
case 0x3712:
case 0x3713:
case 0x3714:
case 0x3715:
case 0x3716:
case 0x3717:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3718:
case 0x3719:
case 0x371a:
case 0x371b:
case 0x371c:
case 0x371d:
case 0x371e:
case 0x371f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3720:
case 0x3721:
case 0x3722:
case 0x3723:
case 0x3724:
case 0x3725:
case 0x3726:
case 0x3727:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3728:
case 0x3729:
case 0x372a:
case 0x372b:
case 0x372c:
case 0x372d:
case 0x372e:
case 0x372f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3730:
case 0x3731:
case 0x3732:
case 0x3733:
case 0x3734:
case 0x3735:
case 0x3736:
case 0x3737:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3738:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3739:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3740:
case 0x3741:
case 0x3742:
case 0x3743:
case 0x3744:
case 0x3745:
case 0x3746:
case 0x3747:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3748:
case 0x3749:
case 0x374a:
case 0x374b:
case 0x374c:
case 0x374d:
case 0x374e:
case 0x374f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3750:
case 0x3751:
case 0x3752:
case 0x3753:
case 0x3754:
case 0x3755:
case 0x3756:
case 0x3757:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3758:
case 0x3759:
case 0x375a:
case 0x375b:
case 0x375c:
case 0x375d:
case 0x375e:
case 0x375f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3760:
case 0x3761:
case 0x3762:
case 0x3763:
case 0x3764:
case 0x3765:
case 0x3766:
case 0x3767:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3768:
case 0x3769:
case 0x376a:
case 0x376b:
case 0x376c:
case 0x376d:
case 0x376e:
case 0x376f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3770:
case 0x3771:
case 0x3772:
case 0x3773:
case 0x3774:
case 0x3775:
case 0x3776:
case 0x3777:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3778:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3779:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3780:
case 0x3781:
case 0x3782:
case 0x3783:
case 0x3784:
case 0x3785:
case 0x3786:
case 0x3787:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3788:
case 0x3789:
case 0x378a:
case 0x378b:
case 0x378c:
case 0x378d:
case 0x378e:
case 0x378f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3790:
case 0x3791:
case 0x3792:
case 0x3793:
case 0x3794:
case 0x3795:
case 0x3796:
case 0x3797:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3798:
case 0x3799:
case 0x379a:
case 0x379b:
case 0x379c:
case 0x379d:
case 0x379e:
case 0x379f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x37a0:
case 0x37a1:
case 0x37a2:
case 0x37a3:
case 0x37a4:
case 0x37a5:
case 0x37a6:
case 0x37a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x37a8:
case 0x37a9:
case 0x37aa:
case 0x37ab:
case 0x37ac:
case 0x37ad:
case 0x37ae:
case 0x37af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x37b0:
case 0x37b1:
case 0x37b2:
case 0x37b3:
case 0x37b4:
case 0x37b5:
case 0x37b6:
case 0x37b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x37b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x37b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x37c0:
case 0x37c1:
case 0x37c2:
case 0x37c3:
case 0x37c4:
case 0x37c5:
case 0x37c6:
case 0x37c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37c8:
case 0x37c9:
case 0x37ca:
case 0x37cb:
case 0x37cc:
case 0x37cd:
case 0x37ce:
case 0x37cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37d0:
case 0x37d1:
case 0x37d2:
case 0x37d3:
case 0x37d4:
case 0x37d5:
case 0x37d6:
case 0x37d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37d8:
case 0x37d9:
case 0x37da:
case 0x37db:
case 0x37dc:
case 0x37dd:
case 0x37de:
case 0x37df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37e0:
case 0x37e1:
case 0x37e2:
case 0x37e3:
case 0x37e4:
case 0x37e5:
case 0x37e6:
case 0x37e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37e8:
case 0x37e9:
case 0x37ea:
case 0x37eb:
case 0x37ec:
case 0x37ed:
case 0x37ee:
case 0x37ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37f0:
case 0x37f1:
case 0x37f2:
case 0x37f3:
case 0x37f4:
case 0x37f5:
case 0x37f6:
case 0x37f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x37fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3800:
case 0x3801:
case 0x3802:
case 0x3803:
case 0x3804:
case 0x3805:
case 0x3806:
case 0x3807:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3810:
case 0x3811:
case 0x3812:
case 0x3813:
case 0x3814:
case 0x3815:
case 0x3816:
case 0x3817:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3818:
case 0x3819:
case 0x381a:
case 0x381b:
case 0x381c:
case 0x381d:
case 0x381e:
case 0x381f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3820:
case 0x3821:
case 0x3822:
case 0x3823:
case 0x3824:
case 0x3825:
case 0x3826:
case 0x3827:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3828:
case 0x3829:
case 0x382a:
case 0x382b:
case 0x382c:
case 0x382d:
case 0x382e:
case 0x382f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3830:
case 0x3831:
case 0x3832:
case 0x3833:
case 0x3834:
case 0x3835:
case 0x3836:
case 0x3837:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3838:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3839:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x383a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x383b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x383c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3840:
case 0x3841:
case 0x3842:
case 0x3843:
case 0x3844:
case 0x3845:
case 0x3846:
case 0x3847:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3848:
case 0x3849:
case 0x384a:
case 0x384b:
case 0x384c:
case 0x384d:
case 0x384e:
case 0x384f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3850:
case 0x3851:
case 0x3852:
case 0x3853:
case 0x3854:
case 0x3855:
case 0x3856:
case 0x3857:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3858:
case 0x3859:
case 0x385a:
case 0x385b:
case 0x385c:
case 0x385d:
case 0x385e:
case 0x385f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3860:
case 0x3861:
case 0x3862:
case 0x3863:
case 0x3864:
case 0x3865:
case 0x3866:
case 0x3867:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3868:
case 0x3869:
case 0x386a:
case 0x386b:
case 0x386c:
case 0x386d:
case 0x386e:
case 0x386f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3870:
case 0x3871:
case 0x3872:
case 0x3873:
case 0x3874:
case 0x3875:
case 0x3876:
case 0x3877:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3878:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3879:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x387a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x387b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x387c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3880:
case 0x3881:
case 0x3882:
case 0x3883:
case 0x3884:
case 0x3885:
case 0x3886:
case 0x3887:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3888:
case 0x3889:
case 0x388a:
case 0x388b:
case 0x388c:
case 0x388d:
case 0x388e:
case 0x388f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3890:
case 0x3891:
case 0x3892:
case 0x3893:
case 0x3894:
case 0x3895:
case 0x3896:
case 0x3897:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3898:
case 0x3899:
case 0x389a:
case 0x389b:
case 0x389c:
case 0x389d:
case 0x389e:
case 0x389f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38a0:
case 0x38a1:
case 0x38a2:
case 0x38a3:
case 0x38a4:
case 0x38a5:
case 0x38a6:
case 0x38a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38a8:
case 0x38a9:
case 0x38aa:
case 0x38ab:
case 0x38ac:
case 0x38ad:
case 0x38ae:
case 0x38af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38b0:
case 0x38b1:
case 0x38b2:
case 0x38b3:
case 0x38b4:
case 0x38b5:
case 0x38b6:
case 0x38b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38c0:
case 0x38c1:
case 0x38c2:
case 0x38c3:
case 0x38c4:
case 0x38c5:
case 0x38c6:
case 0x38c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38c8:
case 0x38c9:
case 0x38ca:
case 0x38cb:
case 0x38cc:
case 0x38cd:
case 0x38ce:
case 0x38cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38d0:
case 0x38d1:
case 0x38d2:
case 0x38d3:
case 0x38d4:
case 0x38d5:
case 0x38d6:
case 0x38d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38d8:
case 0x38d9:
case 0x38da:
case 0x38db:
case 0x38dc:
case 0x38dd:
case 0x38de:
case 0x38df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38e0:
case 0x38e1:
case 0x38e2:
case 0x38e3:
case 0x38e4:
case 0x38e5:
case 0x38e6:
case 0x38e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38e8:
case 0x38e9:
case 0x38ea:
case 0x38eb:
case 0x38ec:
case 0x38ed:
case 0x38ee:
case 0x38ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38f0:
case 0x38f1:
case 0x38f2:
case 0x38f3:
case 0x38f4:
case 0x38f5:
case 0x38f6:
case 0x38f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x38fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3900:
case 0x3901:
case 0x3902:
case 0x3903:
case 0x3904:
case 0x3905:
case 0x3906:
case 0x3907:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3908:
case 0x3909:
case 0x390a:
case 0x390b:
case 0x390c:
case 0x390d:
case 0x390e:
case 0x390f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3910:
case 0x3911:
case 0x3912:
case 0x3913:
case 0x3914:
case 0x3915:
case 0x3916:
case 0x3917:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3918:
case 0x3919:
case 0x391a:
case 0x391b:
case 0x391c:
case 0x391d:
case 0x391e:
case 0x391f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3920:
case 0x3921:
case 0x3922:
case 0x3923:
case 0x3924:
case 0x3925:
case 0x3926:
case 0x3927:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3928:
case 0x3929:
case 0x392a:
case 0x392b:
case 0x392c:
case 0x392d:
case 0x392e:
case 0x392f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3930:
case 0x3931:
case 0x3932:
case 0x3933:
case 0x3934:
case 0x3935:
case 0x3936:
case 0x3937:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3938:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3939:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3940:
case 0x3941:
case 0x3942:
case 0x3943:
case 0x3944:
case 0x3945:
case 0x3946:
case 0x3947:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3948:
case 0x3949:
case 0x394a:
case 0x394b:
case 0x394c:
case 0x394d:
case 0x394e:
case 0x394f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3950:
case 0x3951:
case 0x3952:
case 0x3953:
case 0x3954:
case 0x3955:
case 0x3956:
case 0x3957:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3958:
case 0x3959:
case 0x395a:
case 0x395b:
case 0x395c:
case 0x395d:
case 0x395e:
case 0x395f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3960:
case 0x3961:
case 0x3962:
case 0x3963:
case 0x3964:
case 0x3965:
case 0x3966:
case 0x3967:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3968:
case 0x3969:
case 0x396a:
case 0x396b:
case 0x396c:
case 0x396d:
case 0x396e:
case 0x396f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3970:
case 0x3971:
case 0x3972:
case 0x3973:
case 0x3974:
case 0x3975:
case 0x3976:
case 0x3977:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3978:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3979:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3980:
case 0x3981:
case 0x3982:
case 0x3983:
case 0x3984:
case 0x3985:
case 0x3986:
case 0x3987:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3988:
case 0x3989:
case 0x398a:
case 0x398b:
case 0x398c:
case 0x398d:
case 0x398e:
case 0x398f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3990:
case 0x3991:
case 0x3992:
case 0x3993:
case 0x3994:
case 0x3995:
case 0x3996:
case 0x3997:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3998:
case 0x3999:
case 0x399a:
case 0x399b:
case 0x399c:
case 0x399d:
case 0x399e:
case 0x399f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x39a0:
case 0x39a1:
case 0x39a2:
case 0x39a3:
case 0x39a4:
case 0x39a5:
case 0x39a6:
case 0x39a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x39a8:
case 0x39a9:
case 0x39aa:
case 0x39ab:
case 0x39ac:
case 0x39ad:
case 0x39ae:
case 0x39af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x39b0:
case 0x39b1:
case 0x39b2:
case 0x39b3:
case 0x39b4:
case 0x39b5:
case 0x39b6:
case 0x39b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x39b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x39b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x39c0:
case 0x39c1:
case 0x39c2:
case 0x39c3:
case 0x39c4:
case 0x39c5:
case 0x39c6:
case 0x39c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39c8:
case 0x39c9:
case 0x39ca:
case 0x39cb:
case 0x39cc:
case 0x39cd:
case 0x39ce:
case 0x39cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39d0:
case 0x39d1:
case 0x39d2:
case 0x39d3:
case 0x39d4:
case 0x39d5:
case 0x39d6:
case 0x39d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39d8:
case 0x39d9:
case 0x39da:
case 0x39db:
case 0x39dc:
case 0x39dd:
case 0x39de:
case 0x39df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39e0:
case 0x39e1:
case 0x39e2:
case 0x39e3:
case 0x39e4:
case 0x39e5:
case 0x39e6:
case 0x39e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39e8:
case 0x39e9:
case 0x39ea:
case 0x39eb:
case 0x39ec:
case 0x39ed:
case 0x39ee:
case 0x39ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39f0:
case 0x39f1:
case 0x39f2:
case 0x39f3:
case 0x39f4:
case 0x39f5:
case 0x39f6:
case 0x39f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x39fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a00:
case 0x3a01:
case 0x3a02:
case 0x3a03:
case 0x3a04:
case 0x3a05:
case 0x3a06:
case 0x3a07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a10:
case 0x3a11:
case 0x3a12:
case 0x3a13:
case 0x3a14:
case 0x3a15:
case 0x3a16:
case 0x3a17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a18:
case 0x3a19:
case 0x3a1a:
case 0x3a1b:
case 0x3a1c:
case 0x3a1d:
case 0x3a1e:
case 0x3a1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a20:
case 0x3a21:
case 0x3a22:
case 0x3a23:
case 0x3a24:
case 0x3a25:
case 0x3a26:
case 0x3a27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a28:
case 0x3a29:
case 0x3a2a:
case 0x3a2b:
case 0x3a2c:
case 0x3a2d:
case 0x3a2e:
case 0x3a2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a30:
case 0x3a31:
case 0x3a32:
case 0x3a33:
case 0x3a34:
case 0x3a35:
case 0x3a36:
case 0x3a37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a40:
case 0x3a41:
case 0x3a42:
case 0x3a43:
case 0x3a44:
case 0x3a45:
case 0x3a46:
case 0x3a47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a48:
case 0x3a49:
case 0x3a4a:
case 0x3a4b:
case 0x3a4c:
case 0x3a4d:
case 0x3a4e:
case 0x3a4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a50:
case 0x3a51:
case 0x3a52:
case 0x3a53:
case 0x3a54:
case 0x3a55:
case 0x3a56:
case 0x3a57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a58:
case 0x3a59:
case 0x3a5a:
case 0x3a5b:
case 0x3a5c:
case 0x3a5d:
case 0x3a5e:
case 0x3a5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a60:
case 0x3a61:
case 0x3a62:
case 0x3a63:
case 0x3a64:
case 0x3a65:
case 0x3a66:
case 0x3a67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a68:
case 0x3a69:
case 0x3a6a:
case 0x3a6b:
case 0x3a6c:
case 0x3a6d:
case 0x3a6e:
case 0x3a6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a70:
case 0x3a71:
case 0x3a72:
case 0x3a73:
case 0x3a74:
case 0x3a75:
case 0x3a76:
case 0x3a77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a80:
case 0x3a81:
case 0x3a82:
case 0x3a83:
case 0x3a84:
case 0x3a85:
case 0x3a86:
case 0x3a87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a88:
case 0x3a89:
case 0x3a8a:
case 0x3a8b:
case 0x3a8c:
case 0x3a8d:
case 0x3a8e:
case 0x3a8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a90:
case 0x3a91:
case 0x3a92:
case 0x3a93:
case 0x3a94:
case 0x3a95:
case 0x3a96:
case 0x3a97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3a98:
case 0x3a99:
case 0x3a9a:
case 0x3a9b:
case 0x3a9c:
case 0x3a9d:
case 0x3a9e:
case 0x3a9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3aa0:
case 0x3aa1:
case 0x3aa2:
case 0x3aa3:
case 0x3aa4:
case 0x3aa5:
case 0x3aa6:
case 0x3aa7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3aa8:
case 0x3aa9:
case 0x3aaa:
case 0x3aab:
case 0x3aac:
case 0x3aad:
case 0x3aae:
case 0x3aaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ab0:
case 0x3ab1:
case 0x3ab2:
case 0x3ab3:
case 0x3ab4:
case 0x3ab5:
case 0x3ab6:
case 0x3ab7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ab8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ab9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3aba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3abb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3abc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ac0:
case 0x3ac1:
case 0x3ac2:
case 0x3ac3:
case 0x3ac4:
case 0x3ac5:
case 0x3ac6:
case 0x3ac7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ac8:
case 0x3ac9:
case 0x3aca:
case 0x3acb:
case 0x3acc:
case 0x3acd:
case 0x3ace:
case 0x3acf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ad0:
case 0x3ad1:
case 0x3ad2:
case 0x3ad3:
case 0x3ad4:
case 0x3ad5:
case 0x3ad6:
case 0x3ad7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ad8:
case 0x3ad9:
case 0x3ada:
case 0x3adb:
case 0x3adc:
case 0x3add:
case 0x3ade:
case 0x3adf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ae0:
case 0x3ae1:
case 0x3ae2:
case 0x3ae3:
case 0x3ae4:
case 0x3ae5:
case 0x3ae6:
case 0x3ae7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ae8:
case 0x3ae9:
case 0x3aea:
case 0x3aeb:
case 0x3aec:
case 0x3aed:
case 0x3aee:
case 0x3aef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3af0:
case 0x3af1:
case 0x3af2:
case 0x3af3:
case 0x3af4:
case 0x3af5:
case 0x3af6:
case 0x3af7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3af8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3af9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3afa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3afb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3afc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3b00:
case 0x3b01:
case 0x3b02:
case 0x3b03:
case 0x3b04:
case 0x3b05:
case 0x3b06:
case 0x3b07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b08:
case 0x3b09:
case 0x3b0a:
case 0x3b0b:
case 0x3b0c:
case 0x3b0d:
case 0x3b0e:
case 0x3b0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3b10:
case 0x3b11:
case 0x3b12:
case 0x3b13:
case 0x3b14:
case 0x3b15:
case 0x3b16:
case 0x3b17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b18:
case 0x3b19:
case 0x3b1a:
case 0x3b1b:
case 0x3b1c:
case 0x3b1d:
case 0x3b1e:
case 0x3b1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b20:
case 0x3b21:
case 0x3b22:
case 0x3b23:
case 0x3b24:
case 0x3b25:
case 0x3b26:
case 0x3b27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b28:
case 0x3b29:
case 0x3b2a:
case 0x3b2b:
case 0x3b2c:
case 0x3b2d:
case 0x3b2e:
case 0x3b2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b30:
case 0x3b31:
case 0x3b32:
case 0x3b33:
case 0x3b34:
case 0x3b35:
case 0x3b36:
case 0x3b37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3b39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3b40:
case 0x3b41:
case 0x3b42:
case 0x3b43:
case 0x3b44:
case 0x3b45:
case 0x3b46:
case 0x3b47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b48:
case 0x3b49:
case 0x3b4a:
case 0x3b4b:
case 0x3b4c:
case 0x3b4d:
case 0x3b4e:
case 0x3b4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3b50:
case 0x3b51:
case 0x3b52:
case 0x3b53:
case 0x3b54:
case 0x3b55:
case 0x3b56:
case 0x3b57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b58:
case 0x3b59:
case 0x3b5a:
case 0x3b5b:
case 0x3b5c:
case 0x3b5d:
case 0x3b5e:
case 0x3b5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b60:
case 0x3b61:
case 0x3b62:
case 0x3b63:
case 0x3b64:
case 0x3b65:
case 0x3b66:
case 0x3b67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b68:
case 0x3b69:
case 0x3b6a:
case 0x3b6b:
case 0x3b6c:
case 0x3b6d:
case 0x3b6e:
case 0x3b6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b70:
case 0x3b71:
case 0x3b72:
case 0x3b73:
case 0x3b74:
case 0x3b75:
case 0x3b76:
case 0x3b77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3b79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3b80:
case 0x3b81:
case 0x3b82:
case 0x3b83:
case 0x3b84:
case 0x3b85:
case 0x3b86:
case 0x3b87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b88:
case 0x3b89:
case 0x3b8a:
case 0x3b8b:
case 0x3b8c:
case 0x3b8d:
case 0x3b8e:
case 0x3b8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3b90:
case 0x3b91:
case 0x3b92:
case 0x3b93:
case 0x3b94:
case 0x3b95:
case 0x3b96:
case 0x3b97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3b98:
case 0x3b99:
case 0x3b9a:
case 0x3b9b:
case 0x3b9c:
case 0x3b9d:
case 0x3b9e:
case 0x3b9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3ba0:
case 0x3ba1:
case 0x3ba2:
case 0x3ba3:
case 0x3ba4:
case 0x3ba5:
case 0x3ba6:
case 0x3ba7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3ba8:
case 0x3ba9:
case 0x3baa:
case 0x3bab:
case 0x3bac:
case 0x3bad:
case 0x3bae:
case 0x3baf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3bb0:
case 0x3bb1:
case 0x3bb2:
case 0x3bb3:
case 0x3bb4:
case 0x3bb5:
case 0x3bb6:
case 0x3bb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3bb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3bb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3bc0:
case 0x3bc1:
case 0x3bc2:
case 0x3bc3:
case 0x3bc4:
case 0x3bc5:
case 0x3bc6:
case 0x3bc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bc8:
case 0x3bc9:
case 0x3bca:
case 0x3bcb:
case 0x3bcc:
case 0x3bcd:
case 0x3bce:
case 0x3bcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bd0:
case 0x3bd1:
case 0x3bd2:
case 0x3bd3:
case 0x3bd4:
case 0x3bd5:
case 0x3bd6:
case 0x3bd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bd8:
case 0x3bd9:
case 0x3bda:
case 0x3bdb:
case 0x3bdc:
case 0x3bdd:
case 0x3bde:
case 0x3bdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3be0:
case 0x3be1:
case 0x3be2:
case 0x3be3:
case 0x3be4:
case 0x3be5:
case 0x3be6:
case 0x3be7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3be8:
case 0x3be9:
case 0x3bea:
case 0x3beb:
case 0x3bec:
case 0x3bed:
case 0x3bee:
case 0x3bef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bf0:
case 0x3bf1:
case 0x3bf2:
case 0x3bf3:
case 0x3bf4:
case 0x3bf5:
case 0x3bf6:
case 0x3bf7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bf8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bf9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3bfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c00:
case 0x3c01:
case 0x3c02:
case 0x3c03:
case 0x3c04:
case 0x3c05:
case 0x3c06:
case 0x3c07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c10:
case 0x3c11:
case 0x3c12:
case 0x3c13:
case 0x3c14:
case 0x3c15:
case 0x3c16:
case 0x3c17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c18:
case 0x3c19:
case 0x3c1a:
case 0x3c1b:
case 0x3c1c:
case 0x3c1d:
case 0x3c1e:
case 0x3c1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c20:
case 0x3c21:
case 0x3c22:
case 0x3c23:
case 0x3c24:
case 0x3c25:
case 0x3c26:
case 0x3c27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c28:
case 0x3c29:
case 0x3c2a:
case 0x3c2b:
case 0x3c2c:
case 0x3c2d:
case 0x3c2e:
case 0x3c2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c30:
case 0x3c31:
case 0x3c32:
case 0x3c33:
case 0x3c34:
case 0x3c35:
case 0x3c36:
case 0x3c37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c40:
case 0x3c41:
case 0x3c42:
case 0x3c43:
case 0x3c44:
case 0x3c45:
case 0x3c46:
case 0x3c47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c48:
case 0x3c49:
case 0x3c4a:
case 0x3c4b:
case 0x3c4c:
case 0x3c4d:
case 0x3c4e:
case 0x3c4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c50:
case 0x3c51:
case 0x3c52:
case 0x3c53:
case 0x3c54:
case 0x3c55:
case 0x3c56:
case 0x3c57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c58:
case 0x3c59:
case 0x3c5a:
case 0x3c5b:
case 0x3c5c:
case 0x3c5d:
case 0x3c5e:
case 0x3c5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c60:
case 0x3c61:
case 0x3c62:
case 0x3c63:
case 0x3c64:
case 0x3c65:
case 0x3c66:
case 0x3c67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c68:
case 0x3c69:
case 0x3c6a:
case 0x3c6b:
case 0x3c6c:
case 0x3c6d:
case 0x3c6e:
case 0x3c6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c70:
case 0x3c71:
case 0x3c72:
case 0x3c73:
case 0x3c74:
case 0x3c75:
case 0x3c76:
case 0x3c77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c80:
case 0x3c81:
case 0x3c82:
case 0x3c83:
case 0x3c84:
case 0x3c85:
case 0x3c86:
case 0x3c87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c88:
case 0x3c89:
case 0x3c8a:
case 0x3c8b:
case 0x3c8c:
case 0x3c8d:
case 0x3c8e:
case 0x3c8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c90:
case 0x3c91:
case 0x3c92:
case 0x3c93:
case 0x3c94:
case 0x3c95:
case 0x3c96:
case 0x3c97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3c98:
case 0x3c99:
case 0x3c9a:
case 0x3c9b:
case 0x3c9c:
case 0x3c9d:
case 0x3c9e:
case 0x3c9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ca0:
case 0x3ca1:
case 0x3ca2:
case 0x3ca3:
case 0x3ca4:
case 0x3ca5:
case 0x3ca6:
case 0x3ca7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ca8:
case 0x3ca9:
case 0x3caa:
case 0x3cab:
case 0x3cac:
case 0x3cad:
case 0x3cae:
case 0x3caf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cb0:
case 0x3cb1:
case 0x3cb2:
case 0x3cb3:
case 0x3cb4:
case 0x3cb5:
case 0x3cb6:
case 0x3cb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cbb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cbc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cc0:
case 0x3cc1:
case 0x3cc2:
case 0x3cc3:
case 0x3cc4:
case 0x3cc5:
case 0x3cc6:
case 0x3cc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cc8:
case 0x3cc9:
case 0x3cca:
case 0x3ccb:
case 0x3ccc:
case 0x3ccd:
case 0x3cce:
case 0x3ccf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cd0:
case 0x3cd1:
case 0x3cd2:
case 0x3cd3:
case 0x3cd4:
case 0x3cd5:
case 0x3cd6:
case 0x3cd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cd8:
case 0x3cd9:
case 0x3cda:
case 0x3cdb:
case 0x3cdc:
case 0x3cdd:
case 0x3cde:
case 0x3cdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ce0:
case 0x3ce1:
case 0x3ce2:
case 0x3ce3:
case 0x3ce4:
case 0x3ce5:
case 0x3ce6:
case 0x3ce7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ce8:
case 0x3ce9:
case 0x3cea:
case 0x3ceb:
case 0x3cec:
case 0x3ced:
case 0x3cee:
case 0x3cef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cf0:
case 0x3cf1:
case 0x3cf2:
case 0x3cf3:
case 0x3cf4:
case 0x3cf5:
case 0x3cf6:
case 0x3cf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cf9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cfa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cfb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3cfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3d00:
case 0x3d01:
case 0x3d02:
case 0x3d03:
case 0x3d04:
case 0x3d05:
case 0x3d06:
case 0x3d07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d08:
case 0x3d09:
case 0x3d0a:
case 0x3d0b:
case 0x3d0c:
case 0x3d0d:
case 0x3d0e:
case 0x3d0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3d10:
case 0x3d11:
case 0x3d12:
case 0x3d13:
case 0x3d14:
case 0x3d15:
case 0x3d16:
case 0x3d17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d18:
case 0x3d19:
case 0x3d1a:
case 0x3d1b:
case 0x3d1c:
case 0x3d1d:
case 0x3d1e:
case 0x3d1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d20:
case 0x3d21:
case 0x3d22:
case 0x3d23:
case 0x3d24:
case 0x3d25:
case 0x3d26:
case 0x3d27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d28:
case 0x3d29:
case 0x3d2a:
case 0x3d2b:
case 0x3d2c:
case 0x3d2d:
case 0x3d2e:
case 0x3d2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d30:
case 0x3d31:
case 0x3d32:
case 0x3d33:
case 0x3d34:
case 0x3d35:
case 0x3d36:
case 0x3d37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3d39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3d40:
case 0x3d41:
case 0x3d42:
case 0x3d43:
case 0x3d44:
case 0x3d45:
case 0x3d46:
case 0x3d47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d48:
case 0x3d49:
case 0x3d4a:
case 0x3d4b:
case 0x3d4c:
case 0x3d4d:
case 0x3d4e:
case 0x3d4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3d50:
case 0x3d51:
case 0x3d52:
case 0x3d53:
case 0x3d54:
case 0x3d55:
case 0x3d56:
case 0x3d57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d58:
case 0x3d59:
case 0x3d5a:
case 0x3d5b:
case 0x3d5c:
case 0x3d5d:
case 0x3d5e:
case 0x3d5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d60:
case 0x3d61:
case 0x3d62:
case 0x3d63:
case 0x3d64:
case 0x3d65:
case 0x3d66:
case 0x3d67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d68:
case 0x3d69:
case 0x3d6a:
case 0x3d6b:
case 0x3d6c:
case 0x3d6d:
case 0x3d6e:
case 0x3d6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d70:
case 0x3d71:
case 0x3d72:
case 0x3d73:
case 0x3d74:
case 0x3d75:
case 0x3d76:
case 0x3d77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3d79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3d80:
case 0x3d81:
case 0x3d82:
case 0x3d83:
case 0x3d84:
case 0x3d85:
case 0x3d86:
case 0x3d87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d88:
case 0x3d89:
case 0x3d8a:
case 0x3d8b:
case 0x3d8c:
case 0x3d8d:
case 0x3d8e:
case 0x3d8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3d90:
case 0x3d91:
case 0x3d92:
case 0x3d93:
case 0x3d94:
case 0x3d95:
case 0x3d96:
case 0x3d97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3d98:
case 0x3d99:
case 0x3d9a:
case 0x3d9b:
case 0x3d9c:
case 0x3d9d:
case 0x3d9e:
case 0x3d9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3da0:
case 0x3da1:
case 0x3da2:
case 0x3da3:
case 0x3da4:
case 0x3da5:
case 0x3da6:
case 0x3da7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3da8:
case 0x3da9:
case 0x3daa:
case 0x3dab:
case 0x3dac:
case 0x3dad:
case 0x3dae:
case 0x3daf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3db0:
case 0x3db1:
case 0x3db2:
case 0x3db3:
case 0x3db4:
case 0x3db5:
case 0x3db6:
case 0x3db7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3db8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3db9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3dc0:
case 0x3dc1:
case 0x3dc2:
case 0x3dc3:
case 0x3dc4:
case 0x3dc5:
case 0x3dc6:
case 0x3dc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dc8:
case 0x3dc9:
case 0x3dca:
case 0x3dcb:
case 0x3dcc:
case 0x3dcd:
case 0x3dce:
case 0x3dcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dd0:
case 0x3dd1:
case 0x3dd2:
case 0x3dd3:
case 0x3dd4:
case 0x3dd5:
case 0x3dd6:
case 0x3dd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dd8:
case 0x3dd9:
case 0x3dda:
case 0x3ddb:
case 0x3ddc:
case 0x3ddd:
case 0x3dde:
case 0x3ddf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3de0:
case 0x3de1:
case 0x3de2:
case 0x3de3:
case 0x3de4:
case 0x3de5:
case 0x3de6:
case 0x3de7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3de8:
case 0x3de9:
case 0x3dea:
case 0x3deb:
case 0x3dec:
case 0x3ded:
case 0x3dee:
case 0x3def:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3df0:
case 0x3df1:
case 0x3df2:
case 0x3df3:
case 0x3df4:
case 0x3df5:
case 0x3df6:
case 0x3df7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3df8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3df9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3dfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e00:
case 0x3e01:
case 0x3e02:
case 0x3e03:
case 0x3e04:
case 0x3e05:
case 0x3e06:
case 0x3e07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e10:
case 0x3e11:
case 0x3e12:
case 0x3e13:
case 0x3e14:
case 0x3e15:
case 0x3e16:
case 0x3e17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e18:
case 0x3e19:
case 0x3e1a:
case 0x3e1b:
case 0x3e1c:
case 0x3e1d:
case 0x3e1e:
case 0x3e1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e20:
case 0x3e21:
case 0x3e22:
case 0x3e23:
case 0x3e24:
case 0x3e25:
case 0x3e26:
case 0x3e27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e28:
case 0x3e29:
case 0x3e2a:
case 0x3e2b:
case 0x3e2c:
case 0x3e2d:
case 0x3e2e:
case 0x3e2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e30:
case 0x3e31:
case 0x3e32:
case 0x3e33:
case 0x3e34:
case 0x3e35:
case 0x3e36:
case 0x3e37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e40:
case 0x3e41:
case 0x3e42:
case 0x3e43:
case 0x3e44:
case 0x3e45:
case 0x3e46:
case 0x3e47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e48:
case 0x3e49:
case 0x3e4a:
case 0x3e4b:
case 0x3e4c:
case 0x3e4d:
case 0x3e4e:
case 0x3e4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e50:
case 0x3e51:
case 0x3e52:
case 0x3e53:
case 0x3e54:
case 0x3e55:
case 0x3e56:
case 0x3e57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e58:
case 0x3e59:
case 0x3e5a:
case 0x3e5b:
case 0x3e5c:
case 0x3e5d:
case 0x3e5e:
case 0x3e5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e60:
case 0x3e61:
case 0x3e62:
case 0x3e63:
case 0x3e64:
case 0x3e65:
case 0x3e66:
case 0x3e67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e68:
case 0x3e69:
case 0x3e6a:
case 0x3e6b:
case 0x3e6c:
case 0x3e6d:
case 0x3e6e:
case 0x3e6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e70:
case 0x3e71:
case 0x3e72:
case 0x3e73:
case 0x3e74:
case 0x3e75:
case 0x3e76:
case 0x3e77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e80:
case 0x3e81:
case 0x3e82:
case 0x3e83:
case 0x3e84:
case 0x3e85:
case 0x3e86:
case 0x3e87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e88:
case 0x3e89:
case 0x3e8a:
case 0x3e8b:
case 0x3e8c:
case 0x3e8d:
case 0x3e8e:
case 0x3e8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e90:
case 0x3e91:
case 0x3e92:
case 0x3e93:
case 0x3e94:
case 0x3e95:
case 0x3e96:
case 0x3e97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3e98:
case 0x3e99:
case 0x3e9a:
case 0x3e9b:
case 0x3e9c:
case 0x3e9d:
case 0x3e9e:
case 0x3e9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ea0:
case 0x3ea1:
case 0x3ea2:
case 0x3ea3:
case 0x3ea4:
case 0x3ea5:
case 0x3ea6:
case 0x3ea7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ea8:
case 0x3ea9:
case 0x3eaa:
case 0x3eab:
case 0x3eac:
case 0x3ead:
case 0x3eae:
case 0x3eaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3eb0:
case 0x3eb1:
case 0x3eb2:
case 0x3eb3:
case 0x3eb4:
case 0x3eb5:
case 0x3eb6:
case 0x3eb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3eb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3eb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3eba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ebb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ebc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ec0:
case 0x3ec1:
case 0x3ec2:
case 0x3ec3:
case 0x3ec4:
case 0x3ec5:
case 0x3ec6:
case 0x3ec7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ec8:
case 0x3ec9:
case 0x3eca:
case 0x3ecb:
case 0x3ecc:
case 0x3ecd:
case 0x3ece:
case 0x3ecf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ed0:
case 0x3ed1:
case 0x3ed2:
case 0x3ed3:
case 0x3ed4:
case 0x3ed5:
case 0x3ed6:
case 0x3ed7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ed8:
case 0x3ed9:
case 0x3eda:
case 0x3edb:
case 0x3edc:
case 0x3edd:
case 0x3ede:
case 0x3edf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ee0:
case 0x3ee1:
case 0x3ee2:
case 0x3ee3:
case 0x3ee4:
case 0x3ee5:
case 0x3ee6:
case 0x3ee7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ee8:
case 0x3ee9:
case 0x3eea:
case 0x3eeb:
case 0x3eec:
case 0x3eed:
case 0x3eee:
case 0x3eef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ef0:
case 0x3ef1:
case 0x3ef2:
case 0x3ef3:
case 0x3ef4:
case 0x3ef5:
case 0x3ef6:
case 0x3ef7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ef8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ef9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3efa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3efb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3efc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3f00:
case 0x3f01:
case 0x3f02:
case 0x3f03:
case 0x3f04:
case 0x3f05:
case 0x3f06:
case 0x3f07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f08:
case 0x3f09:
case 0x3f0a:
case 0x3f0b:
case 0x3f0c:
case 0x3f0d:
case 0x3f0e:
case 0x3f0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3f10:
case 0x3f11:
case 0x3f12:
case 0x3f13:
case 0x3f14:
case 0x3f15:
case 0x3f16:
case 0x3f17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f18:
case 0x3f19:
case 0x3f1a:
case 0x3f1b:
case 0x3f1c:
case 0x3f1d:
case 0x3f1e:
case 0x3f1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f20:
case 0x3f21:
case 0x3f22:
case 0x3f23:
case 0x3f24:
case 0x3f25:
case 0x3f26:
case 0x3f27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f28:
case 0x3f29:
case 0x3f2a:
case 0x3f2b:
case 0x3f2c:
case 0x3f2d:
case 0x3f2e:
case 0x3f2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f30:
case 0x3f31:
case 0x3f32:
case 0x3f33:
case 0x3f34:
case 0x3f35:
case 0x3f36:
case 0x3f37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3f39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3f40:
case 0x3f41:
case 0x3f42:
case 0x3f43:
case 0x3f44:
case 0x3f45:
case 0x3f46:
case 0x3f47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f48:
case 0x3f49:
case 0x3f4a:
case 0x3f4b:
case 0x3f4c:
case 0x3f4d:
case 0x3f4e:
case 0x3f4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3f50:
case 0x3f51:
case 0x3f52:
case 0x3f53:
case 0x3f54:
case 0x3f55:
case 0x3f56:
case 0x3f57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f58:
case 0x3f59:
case 0x3f5a:
case 0x3f5b:
case 0x3f5c:
case 0x3f5d:
case 0x3f5e:
case 0x3f5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f60:
case 0x3f61:
case 0x3f62:
case 0x3f63:
case 0x3f64:
case 0x3f65:
case 0x3f66:
case 0x3f67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f68:
case 0x3f69:
case 0x3f6a:
case 0x3f6b:
case 0x3f6c:
case 0x3f6d:
case 0x3f6e:
case 0x3f6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f70:
case 0x3f71:
case 0x3f72:
case 0x3f73:
case 0x3f74:
case 0x3f75:
case 0x3f76:
case 0x3f77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3f79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3f80:
case 0x3f81:
case 0x3f82:
case 0x3f83:
case 0x3f84:
case 0x3f85:
case 0x3f86:
case 0x3f87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, DATA_REG_DIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f88:
case 0x3f89:
case 0x3f8a:
case 0x3f8b:
case 0x3f8c:
case 0x3f8d:
case 0x3f8e:
case 0x3f8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3f90:
case 0x3f91:
case 0x3f92:
case 0x3f93:
case 0x3f94:
case 0x3f95:
case 0x3f96:
case 0x3f97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3f98:
case 0x3f99:
case 0x3f9a:
case 0x3f9b:
case 0x3f9c:
case 0x3f9d:
case 0x3f9e:
case 0x3f9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3fa0:
case 0x3fa1:
case 0x3fa2:
case 0x3fa3:
case 0x3fa4:
case 0x3fa5:
case 0x3fa6:
case 0x3fa7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3fa8:
case 0x3fa9:
case 0x3faa:
case 0x3fab:
case 0x3fac:
case 0x3fad:
case 0x3fae:
case 0x3faf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3fb0:
case 0x3fb1:
case 0x3fb2:
case 0x3fb3:
case 0x3fb4:
case 0x3fb5:
case 0x3fb6:
case 0x3fb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); EOR(src, dst);
	}
	break;

case 0x3fb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); EOR(src, dst);
	}
	break;

case 0x3fb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); EOR(src, dst);
	}
	break;

case 0x3fc0:
case 0x3fc1:
case 0x3fc2:
case 0x3fc3:
case 0x3fc4:
case 0x3fc5:
case 0x3fc6:
case 0x3fc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3fc8:
case 0x3fc9:
case 0x3fca:
case 0x3fcb:
case 0x3fcc:
case 0x3fcd:
case 0x3fce:
case 0x3fcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3fd0:
case 0x3fd1:
case 0x3fd2:
case 0x3fd3:
case 0x3fd4:
case 0x3fd5:
case 0x3fd6:
case 0x3fd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3fd8:
case 0x3fd9:
case 0x3fda:
case 0x3fdb:
case 0x3fdc:
case 0x3fdd:
case 0x3fde:
case 0x3fdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3fe0:
case 0x3fe1:
case 0x3fe2:
case 0x3fe3:
case 0x3fe4:
case 0x3fe5:
case 0x3fe6:
case 0x3fe7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3fe8:
case 0x3fe9:
case 0x3fea:
case 0x3feb:
case 0x3fec:
case 0x3fed:
case 0x3fee:
case 0x3fef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ff0:
case 0x3ff1:
case 0x3ff2:
case 0x3ff3:
case 0x3ff4:
case 0x3ff5:
case 0x3ff6:
case 0x3ff7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ff8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ff9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ffa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ffb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x3ffc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); CMP(src, dst);
	}
	break;

case 0x4000:
case 0x4001:
case 0x4002:
case 0x4003:
case 0x4004:
case 0x4005:
case 0x4006:
case 0x4007:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4010:
case 0x4011:
case 0x4012:
case 0x4013:
case 0x4014:
case 0x4015:
case 0x4016:
case 0x4017:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4018:
case 0x4019:
case 0x401a:
case 0x401b:
case 0x401c:
case 0x401d:
case 0x401e:
case 0x401f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4020:
case 0x4021:
case 0x4022:
case 0x4023:
case 0x4024:
case 0x4025:
case 0x4026:
case 0x4027:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4028:
case 0x4029:
case 0x402a:
case 0x402b:
case 0x402c:
case 0x402d:
case 0x402e:
case 0x402f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4030:
case 0x4031:
case 0x4032:
case 0x4033:
case 0x4034:
case 0x4035:
case 0x4036:
case 0x4037:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4038:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4039:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x403a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x403b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x403c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4040:
case 0x4041:
case 0x4042:
case 0x4043:
case 0x4044:
case 0x4045:
case 0x4046:
case 0x4047:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4050:
case 0x4051:
case 0x4052:
case 0x4053:
case 0x4054:
case 0x4055:
case 0x4056:
case 0x4057:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4058:
case 0x4059:
case 0x405a:
case 0x405b:
case 0x405c:
case 0x405d:
case 0x405e:
case 0x405f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4060:
case 0x4061:
case 0x4062:
case 0x4063:
case 0x4064:
case 0x4065:
case 0x4066:
case 0x4067:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4068:
case 0x4069:
case 0x406a:
case 0x406b:
case 0x406c:
case 0x406d:
case 0x406e:
case 0x406f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4070:
case 0x4071:
case 0x4072:
case 0x4073:
case 0x4074:
case 0x4075:
case 0x4076:
case 0x4077:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4078:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4079:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x407a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x407b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x407c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4080:
case 0x4081:
case 0x4082:
case 0x4083:
case 0x4084:
case 0x4085:
case 0x4086:
case 0x4087:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4090:
case 0x4091:
case 0x4092:
case 0x4093:
case 0x4094:
case 0x4095:
case 0x4096:
case 0x4097:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4098:
case 0x4099:
case 0x409a:
case 0x409b:
case 0x409c:
case 0x409d:
case 0x409e:
case 0x409f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40a0:
case 0x40a1:
case 0x40a2:
case 0x40a3:
case 0x40a4:
case 0x40a5:
case 0x40a6:
case 0x40a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40a8:
case 0x40a9:
case 0x40aa:
case 0x40ab:
case 0x40ac:
case 0x40ad:
case 0x40ae:
case 0x40af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40b0:
case 0x40b1:
case 0x40b2:
case 0x40b3:
case 0x40b4:
case 0x40b5:
case 0x40b6:
case 0x40b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x40c0:
case 0x40c1:
case 0x40c2:
case 0x40c3:
case 0x40c4:
case 0x40c5:
case 0x40c6:
case 0x40c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40d0:
case 0x40d1:
case 0x40d2:
case 0x40d3:
case 0x40d4:
case 0x40d5:
case 0x40d6:
case 0x40d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40d8:
case 0x40d9:
case 0x40da:
case 0x40db:
case 0x40dc:
case 0x40dd:
case 0x40de:
case 0x40df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40e0:
case 0x40e1:
case 0x40e2:
case 0x40e3:
case 0x40e4:
case 0x40e5:
case 0x40e6:
case 0x40e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40e8:
case 0x40e9:
case 0x40ea:
case 0x40eb:
case 0x40ec:
case 0x40ed:
case 0x40ee:
case 0x40ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40f0:
case 0x40f1:
case 0x40f2:
case 0x40f3:
case 0x40f4:
case 0x40f5:
case 0x40f6:
case 0x40f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x40f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x40f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x40fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x40fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x40fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4100:
case 0x4101:
case 0x4102:
case 0x4103:
case 0x4104:
case 0x4105:
case 0x4106:
case 0x4107:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4108:
case 0x4109:
case 0x410a:
case 0x410b:
case 0x410c:
case 0x410d:
case 0x410e:
case 0x410f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4110:
case 0x4111:
case 0x4112:
case 0x4113:
case 0x4114:
case 0x4115:
case 0x4116:
case 0x4117:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4118:
case 0x4119:
case 0x411a:
case 0x411b:
case 0x411c:
case 0x411d:
case 0x411e:
case 0x411f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4120:
case 0x4121:
case 0x4122:
case 0x4123:
case 0x4124:
case 0x4125:
case 0x4126:
case 0x4127:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4128:
case 0x4129:
case 0x412a:
case 0x412b:
case 0x412c:
case 0x412d:
case 0x412e:
case 0x412f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4130:
case 0x4131:
case 0x4132:
case 0x4133:
case 0x4134:
case 0x4135:
case 0x4136:
case 0x4137:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4138:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4139:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4140:
case 0x4141:
case 0x4142:
case 0x4143:
case 0x4144:
case 0x4145:
case 0x4146:
case 0x4147:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4148:
case 0x4149:
case 0x414a:
case 0x414b:
case 0x414c:
case 0x414d:
case 0x414e:
case 0x414f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4150:
case 0x4151:
case 0x4152:
case 0x4153:
case 0x4154:
case 0x4155:
case 0x4156:
case 0x4157:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4158:
case 0x4159:
case 0x415a:
case 0x415b:
case 0x415c:
case 0x415d:
case 0x415e:
case 0x415f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4160:
case 0x4161:
case 0x4162:
case 0x4163:
case 0x4164:
case 0x4165:
case 0x4166:
case 0x4167:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4168:
case 0x4169:
case 0x416a:
case 0x416b:
case 0x416c:
case 0x416d:
case 0x416e:
case 0x416f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4170:
case 0x4171:
case 0x4172:
case 0x4173:
case 0x4174:
case 0x4175:
case 0x4176:
case 0x4177:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4178:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4179:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4188:
case 0x4189:
case 0x418a:
case 0x418b:
case 0x418c:
case 0x418d:
case 0x418e:
case 0x418f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4190:
case 0x4191:
case 0x4192:
case 0x4193:
case 0x4194:
case 0x4195:
case 0x4196:
case 0x4197:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4198:
case 0x4199:
case 0x419a:
case 0x419b:
case 0x419c:
case 0x419d:
case 0x419e:
case 0x419f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x41a0:
case 0x41a1:
case 0x41a2:
case 0x41a3:
case 0x41a4:
case 0x41a5:
case 0x41a6:
case 0x41a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x41a8:
case 0x41a9:
case 0x41aa:
case 0x41ab:
case 0x41ac:
case 0x41ad:
case 0x41ae:
case 0x41af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x41b0:
case 0x41b1:
case 0x41b2:
case 0x41b3:
case 0x41b4:
case 0x41b5:
case 0x41b6:
case 0x41b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x41b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x41b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x41c0:
case 0x41c1:
case 0x41c2:
case 0x41c3:
case 0x41c4:
case 0x41c5:
case 0x41c6:
case 0x41c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41d0:
case 0x41d1:
case 0x41d2:
case 0x41d3:
case 0x41d4:
case 0x41d5:
case 0x41d6:
case 0x41d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41d8:
case 0x41d9:
case 0x41da:
case 0x41db:
case 0x41dc:
case 0x41dd:
case 0x41de:
case 0x41df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41e0:
case 0x41e1:
case 0x41e2:
case 0x41e3:
case 0x41e4:
case 0x41e5:
case 0x41e6:
case 0x41e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41e8:
case 0x41e9:
case 0x41ea:
case 0x41eb:
case 0x41ec:
case 0x41ed:
case 0x41ee:
case 0x41ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41f0:
case 0x41f1:
case 0x41f2:
case 0x41f3:
case 0x41f4:
case 0x41f5:
case 0x41f6:
case 0x41f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x41f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x41f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x41fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x41fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x41fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4200:
case 0x4201:
case 0x4202:
case 0x4203:
case 0x4204:
case 0x4205:
case 0x4206:
case 0x4207:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4210:
case 0x4211:
case 0x4212:
case 0x4213:
case 0x4214:
case 0x4215:
case 0x4216:
case 0x4217:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4218:
case 0x4219:
case 0x421a:
case 0x421b:
case 0x421c:
case 0x421d:
case 0x421e:
case 0x421f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4220:
case 0x4221:
case 0x4222:
case 0x4223:
case 0x4224:
case 0x4225:
case 0x4226:
case 0x4227:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4228:
case 0x4229:
case 0x422a:
case 0x422b:
case 0x422c:
case 0x422d:
case 0x422e:
case 0x422f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4230:
case 0x4231:
case 0x4232:
case 0x4233:
case 0x4234:
case 0x4235:
case 0x4236:
case 0x4237:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4238:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4239:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x423a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x423b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x423c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4240:
case 0x4241:
case 0x4242:
case 0x4243:
case 0x4244:
case 0x4245:
case 0x4246:
case 0x4247:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4250:
case 0x4251:
case 0x4252:
case 0x4253:
case 0x4254:
case 0x4255:
case 0x4256:
case 0x4257:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4258:
case 0x4259:
case 0x425a:
case 0x425b:
case 0x425c:
case 0x425d:
case 0x425e:
case 0x425f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4260:
case 0x4261:
case 0x4262:
case 0x4263:
case 0x4264:
case 0x4265:
case 0x4266:
case 0x4267:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4268:
case 0x4269:
case 0x426a:
case 0x426b:
case 0x426c:
case 0x426d:
case 0x426e:
case 0x426f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4270:
case 0x4271:
case 0x4272:
case 0x4273:
case 0x4274:
case 0x4275:
case 0x4276:
case 0x4277:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4278:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4279:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x427a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x427b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x427c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4280:
case 0x4281:
case 0x4282:
case 0x4283:
case 0x4284:
case 0x4285:
case 0x4286:
case 0x4287:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4290:
case 0x4291:
case 0x4292:
case 0x4293:
case 0x4294:
case 0x4295:
case 0x4296:
case 0x4297:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4298:
case 0x4299:
case 0x429a:
case 0x429b:
case 0x429c:
case 0x429d:
case 0x429e:
case 0x429f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42a0:
case 0x42a1:
case 0x42a2:
case 0x42a3:
case 0x42a4:
case 0x42a5:
case 0x42a6:
case 0x42a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42a8:
case 0x42a9:
case 0x42aa:
case 0x42ab:
case 0x42ac:
case 0x42ad:
case 0x42ae:
case 0x42af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42b0:
case 0x42b1:
case 0x42b2:
case 0x42b3:
case 0x42b4:
case 0x42b5:
case 0x42b6:
case 0x42b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x42c0:
case 0x42c1:
case 0x42c2:
case 0x42c3:
case 0x42c4:
case 0x42c5:
case 0x42c6:
case 0x42c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42d0:
case 0x42d1:
case 0x42d2:
case 0x42d3:
case 0x42d4:
case 0x42d5:
case 0x42d6:
case 0x42d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42d8:
case 0x42d9:
case 0x42da:
case 0x42db:
case 0x42dc:
case 0x42dd:
case 0x42de:
case 0x42df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42e0:
case 0x42e1:
case 0x42e2:
case 0x42e3:
case 0x42e4:
case 0x42e5:
case 0x42e6:
case 0x42e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42e8:
case 0x42e9:
case 0x42ea:
case 0x42eb:
case 0x42ec:
case 0x42ed:
case 0x42ee:
case 0x42ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42f0:
case 0x42f1:
case 0x42f2:
case 0x42f3:
case 0x42f4:
case 0x42f5:
case 0x42f6:
case 0x42f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x42f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x42f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x42fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x42fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x42fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4300:
case 0x4301:
case 0x4302:
case 0x4303:
case 0x4304:
case 0x4305:
case 0x4306:
case 0x4307:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4308:
case 0x4309:
case 0x430a:
case 0x430b:
case 0x430c:
case 0x430d:
case 0x430e:
case 0x430f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4310:
case 0x4311:
case 0x4312:
case 0x4313:
case 0x4314:
case 0x4315:
case 0x4316:
case 0x4317:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4318:
case 0x4319:
case 0x431a:
case 0x431b:
case 0x431c:
case 0x431d:
case 0x431e:
case 0x431f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4320:
case 0x4321:
case 0x4322:
case 0x4323:
case 0x4324:
case 0x4325:
case 0x4326:
case 0x4327:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4328:
case 0x4329:
case 0x432a:
case 0x432b:
case 0x432c:
case 0x432d:
case 0x432e:
case 0x432f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4330:
case 0x4331:
case 0x4332:
case 0x4333:
case 0x4334:
case 0x4335:
case 0x4336:
case 0x4337:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4338:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4339:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4340:
case 0x4341:
case 0x4342:
case 0x4343:
case 0x4344:
case 0x4345:
case 0x4346:
case 0x4347:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4348:
case 0x4349:
case 0x434a:
case 0x434b:
case 0x434c:
case 0x434d:
case 0x434e:
case 0x434f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4350:
case 0x4351:
case 0x4352:
case 0x4353:
case 0x4354:
case 0x4355:
case 0x4356:
case 0x4357:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4358:
case 0x4359:
case 0x435a:
case 0x435b:
case 0x435c:
case 0x435d:
case 0x435e:
case 0x435f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4360:
case 0x4361:
case 0x4362:
case 0x4363:
case 0x4364:
case 0x4365:
case 0x4366:
case 0x4367:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4368:
case 0x4369:
case 0x436a:
case 0x436b:
case 0x436c:
case 0x436d:
case 0x436e:
case 0x436f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4370:
case 0x4371:
case 0x4372:
case 0x4373:
case 0x4374:
case 0x4375:
case 0x4376:
case 0x4377:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4378:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4379:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4388:
case 0x4389:
case 0x438a:
case 0x438b:
case 0x438c:
case 0x438d:
case 0x438e:
case 0x438f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4390:
case 0x4391:
case 0x4392:
case 0x4393:
case 0x4394:
case 0x4395:
case 0x4396:
case 0x4397:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4398:
case 0x4399:
case 0x439a:
case 0x439b:
case 0x439c:
case 0x439d:
case 0x439e:
case 0x439f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x43a0:
case 0x43a1:
case 0x43a2:
case 0x43a3:
case 0x43a4:
case 0x43a5:
case 0x43a6:
case 0x43a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x43a8:
case 0x43a9:
case 0x43aa:
case 0x43ab:
case 0x43ac:
case 0x43ad:
case 0x43ae:
case 0x43af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x43b0:
case 0x43b1:
case 0x43b2:
case 0x43b3:
case 0x43b4:
case 0x43b5:
case 0x43b6:
case 0x43b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x43b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x43b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x43c0:
case 0x43c1:
case 0x43c2:
case 0x43c3:
case 0x43c4:
case 0x43c5:
case 0x43c6:
case 0x43c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43d0:
case 0x43d1:
case 0x43d2:
case 0x43d3:
case 0x43d4:
case 0x43d5:
case 0x43d6:
case 0x43d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43d8:
case 0x43d9:
case 0x43da:
case 0x43db:
case 0x43dc:
case 0x43dd:
case 0x43de:
case 0x43df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43e0:
case 0x43e1:
case 0x43e2:
case 0x43e3:
case 0x43e4:
case 0x43e5:
case 0x43e6:
case 0x43e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43e8:
case 0x43e9:
case 0x43ea:
case 0x43eb:
case 0x43ec:
case 0x43ed:
case 0x43ee:
case 0x43ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43f0:
case 0x43f1:
case 0x43f2:
case 0x43f3:
case 0x43f4:
case 0x43f5:
case 0x43f6:
case 0x43f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x43f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x43f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x43fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x43fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x43fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4400:
case 0x4401:
case 0x4402:
case 0x4403:
case 0x4404:
case 0x4405:
case 0x4406:
case 0x4407:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4410:
case 0x4411:
case 0x4412:
case 0x4413:
case 0x4414:
case 0x4415:
case 0x4416:
case 0x4417:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4418:
case 0x4419:
case 0x441a:
case 0x441b:
case 0x441c:
case 0x441d:
case 0x441e:
case 0x441f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4420:
case 0x4421:
case 0x4422:
case 0x4423:
case 0x4424:
case 0x4425:
case 0x4426:
case 0x4427:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4428:
case 0x4429:
case 0x442a:
case 0x442b:
case 0x442c:
case 0x442d:
case 0x442e:
case 0x442f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4430:
case 0x4431:
case 0x4432:
case 0x4433:
case 0x4434:
case 0x4435:
case 0x4436:
case 0x4437:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4438:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4439:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x443a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x443b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x443c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4440:
case 0x4441:
case 0x4442:
case 0x4443:
case 0x4444:
case 0x4445:
case 0x4446:
case 0x4447:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4450:
case 0x4451:
case 0x4452:
case 0x4453:
case 0x4454:
case 0x4455:
case 0x4456:
case 0x4457:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4458:
case 0x4459:
case 0x445a:
case 0x445b:
case 0x445c:
case 0x445d:
case 0x445e:
case 0x445f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4460:
case 0x4461:
case 0x4462:
case 0x4463:
case 0x4464:
case 0x4465:
case 0x4466:
case 0x4467:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4468:
case 0x4469:
case 0x446a:
case 0x446b:
case 0x446c:
case 0x446d:
case 0x446e:
case 0x446f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4470:
case 0x4471:
case 0x4472:
case 0x4473:
case 0x4474:
case 0x4475:
case 0x4476:
case 0x4477:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4478:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4479:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x447a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x447b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x447c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4480:
case 0x4481:
case 0x4482:
case 0x4483:
case 0x4484:
case 0x4485:
case 0x4486:
case 0x4487:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4490:
case 0x4491:
case 0x4492:
case 0x4493:
case 0x4494:
case 0x4495:
case 0x4496:
case 0x4497:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4498:
case 0x4499:
case 0x449a:
case 0x449b:
case 0x449c:
case 0x449d:
case 0x449e:
case 0x449f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44a0:
case 0x44a1:
case 0x44a2:
case 0x44a3:
case 0x44a4:
case 0x44a5:
case 0x44a6:
case 0x44a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44a8:
case 0x44a9:
case 0x44aa:
case 0x44ab:
case 0x44ac:
case 0x44ad:
case 0x44ae:
case 0x44af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44b0:
case 0x44b1:
case 0x44b2:
case 0x44b3:
case 0x44b4:
case 0x44b5:
case 0x44b6:
case 0x44b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x44c0:
case 0x44c1:
case 0x44c2:
case 0x44c3:
case 0x44c4:
case 0x44c5:
case 0x44c6:
case 0x44c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44d0:
case 0x44d1:
case 0x44d2:
case 0x44d3:
case 0x44d4:
case 0x44d5:
case 0x44d6:
case 0x44d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44d8:
case 0x44d9:
case 0x44da:
case 0x44db:
case 0x44dc:
case 0x44dd:
case 0x44de:
case 0x44df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44e0:
case 0x44e1:
case 0x44e2:
case 0x44e3:
case 0x44e4:
case 0x44e5:
case 0x44e6:
case 0x44e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44e8:
case 0x44e9:
case 0x44ea:
case 0x44eb:
case 0x44ec:
case 0x44ed:
case 0x44ee:
case 0x44ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44f0:
case 0x44f1:
case 0x44f2:
case 0x44f3:
case 0x44f4:
case 0x44f5:
case 0x44f6:
case 0x44f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x44f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x44f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x44fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x44fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x44fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4500:
case 0x4501:
case 0x4502:
case 0x4503:
case 0x4504:
case 0x4505:
case 0x4506:
case 0x4507:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4508:
case 0x4509:
case 0x450a:
case 0x450b:
case 0x450c:
case 0x450d:
case 0x450e:
case 0x450f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4510:
case 0x4511:
case 0x4512:
case 0x4513:
case 0x4514:
case 0x4515:
case 0x4516:
case 0x4517:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4518:
case 0x4519:
case 0x451a:
case 0x451b:
case 0x451c:
case 0x451d:
case 0x451e:
case 0x451f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4520:
case 0x4521:
case 0x4522:
case 0x4523:
case 0x4524:
case 0x4525:
case 0x4526:
case 0x4527:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4528:
case 0x4529:
case 0x452a:
case 0x452b:
case 0x452c:
case 0x452d:
case 0x452e:
case 0x452f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4530:
case 0x4531:
case 0x4532:
case 0x4533:
case 0x4534:
case 0x4535:
case 0x4536:
case 0x4537:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4538:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4539:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4540:
case 0x4541:
case 0x4542:
case 0x4543:
case 0x4544:
case 0x4545:
case 0x4546:
case 0x4547:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4548:
case 0x4549:
case 0x454a:
case 0x454b:
case 0x454c:
case 0x454d:
case 0x454e:
case 0x454f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4550:
case 0x4551:
case 0x4552:
case 0x4553:
case 0x4554:
case 0x4555:
case 0x4556:
case 0x4557:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4558:
case 0x4559:
case 0x455a:
case 0x455b:
case 0x455c:
case 0x455d:
case 0x455e:
case 0x455f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4560:
case 0x4561:
case 0x4562:
case 0x4563:
case 0x4564:
case 0x4565:
case 0x4566:
case 0x4567:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4568:
case 0x4569:
case 0x456a:
case 0x456b:
case 0x456c:
case 0x456d:
case 0x456e:
case 0x456f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4570:
case 0x4571:
case 0x4572:
case 0x4573:
case 0x4574:
case 0x4575:
case 0x4576:
case 0x4577:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4578:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4579:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4588:
case 0x4589:
case 0x458a:
case 0x458b:
case 0x458c:
case 0x458d:
case 0x458e:
case 0x458f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4590:
case 0x4591:
case 0x4592:
case 0x4593:
case 0x4594:
case 0x4595:
case 0x4596:
case 0x4597:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4598:
case 0x4599:
case 0x459a:
case 0x459b:
case 0x459c:
case 0x459d:
case 0x459e:
case 0x459f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x45a0:
case 0x45a1:
case 0x45a2:
case 0x45a3:
case 0x45a4:
case 0x45a5:
case 0x45a6:
case 0x45a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x45a8:
case 0x45a9:
case 0x45aa:
case 0x45ab:
case 0x45ac:
case 0x45ad:
case 0x45ae:
case 0x45af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x45b0:
case 0x45b1:
case 0x45b2:
case 0x45b3:
case 0x45b4:
case 0x45b5:
case 0x45b6:
case 0x45b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x45b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x45b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x45c0:
case 0x45c1:
case 0x45c2:
case 0x45c3:
case 0x45c4:
case 0x45c5:
case 0x45c6:
case 0x45c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45d0:
case 0x45d1:
case 0x45d2:
case 0x45d3:
case 0x45d4:
case 0x45d5:
case 0x45d6:
case 0x45d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45d8:
case 0x45d9:
case 0x45da:
case 0x45db:
case 0x45dc:
case 0x45dd:
case 0x45de:
case 0x45df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45e0:
case 0x45e1:
case 0x45e2:
case 0x45e3:
case 0x45e4:
case 0x45e5:
case 0x45e6:
case 0x45e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45e8:
case 0x45e9:
case 0x45ea:
case 0x45eb:
case 0x45ec:
case 0x45ed:
case 0x45ee:
case 0x45ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45f0:
case 0x45f1:
case 0x45f2:
case 0x45f3:
case 0x45f4:
case 0x45f5:
case 0x45f6:
case 0x45f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x45f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x45f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x45fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x45fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x45fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4600:
case 0x4601:
case 0x4602:
case 0x4603:
case 0x4604:
case 0x4605:
case 0x4606:
case 0x4607:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4610:
case 0x4611:
case 0x4612:
case 0x4613:
case 0x4614:
case 0x4615:
case 0x4616:
case 0x4617:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4618:
case 0x4619:
case 0x461a:
case 0x461b:
case 0x461c:
case 0x461d:
case 0x461e:
case 0x461f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4620:
case 0x4621:
case 0x4622:
case 0x4623:
case 0x4624:
case 0x4625:
case 0x4626:
case 0x4627:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4628:
case 0x4629:
case 0x462a:
case 0x462b:
case 0x462c:
case 0x462d:
case 0x462e:
case 0x462f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4630:
case 0x4631:
case 0x4632:
case 0x4633:
case 0x4634:
case 0x4635:
case 0x4636:
case 0x4637:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4638:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4639:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x463a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x463b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x463c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4640:
case 0x4641:
case 0x4642:
case 0x4643:
case 0x4644:
case 0x4645:
case 0x4646:
case 0x4647:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4650:
case 0x4651:
case 0x4652:
case 0x4653:
case 0x4654:
case 0x4655:
case 0x4656:
case 0x4657:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4658:
case 0x4659:
case 0x465a:
case 0x465b:
case 0x465c:
case 0x465d:
case 0x465e:
case 0x465f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4660:
case 0x4661:
case 0x4662:
case 0x4663:
case 0x4664:
case 0x4665:
case 0x4666:
case 0x4667:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4668:
case 0x4669:
case 0x466a:
case 0x466b:
case 0x466c:
case 0x466d:
case 0x466e:
case 0x466f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4670:
case 0x4671:
case 0x4672:
case 0x4673:
case 0x4674:
case 0x4675:
case 0x4676:
case 0x4677:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4678:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4679:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x467a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x467b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x467c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4680:
case 0x4681:
case 0x4682:
case 0x4683:
case 0x4684:
case 0x4685:
case 0x4686:
case 0x4687:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4690:
case 0x4691:
case 0x4692:
case 0x4693:
case 0x4694:
case 0x4695:
case 0x4696:
case 0x4697:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4698:
case 0x4699:
case 0x469a:
case 0x469b:
case 0x469c:
case 0x469d:
case 0x469e:
case 0x469f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46a0:
case 0x46a1:
case 0x46a2:
case 0x46a3:
case 0x46a4:
case 0x46a5:
case 0x46a6:
case 0x46a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46a8:
case 0x46a9:
case 0x46aa:
case 0x46ab:
case 0x46ac:
case 0x46ad:
case 0x46ae:
case 0x46af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46b0:
case 0x46b1:
case 0x46b2:
case 0x46b3:
case 0x46b4:
case 0x46b5:
case 0x46b6:
case 0x46b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x46c0:
case 0x46c1:
case 0x46c2:
case 0x46c3:
case 0x46c4:
case 0x46c5:
case 0x46c6:
case 0x46c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46d0:
case 0x46d1:
case 0x46d2:
case 0x46d3:
case 0x46d4:
case 0x46d5:
case 0x46d6:
case 0x46d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46d8:
case 0x46d9:
case 0x46da:
case 0x46db:
case 0x46dc:
case 0x46dd:
case 0x46de:
case 0x46df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46e0:
case 0x46e1:
case 0x46e2:
case 0x46e3:
case 0x46e4:
case 0x46e5:
case 0x46e6:
case 0x46e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46e8:
case 0x46e9:
case 0x46ea:
case 0x46eb:
case 0x46ec:
case 0x46ed:
case 0x46ee:
case 0x46ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46f0:
case 0x46f1:
case 0x46f2:
case 0x46f3:
case 0x46f4:
case 0x46f5:
case 0x46f6:
case 0x46f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x46f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x46f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x46fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x46fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x46fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4700:
case 0x4701:
case 0x4702:
case 0x4703:
case 0x4704:
case 0x4705:
case 0x4706:
case 0x4707:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4708:
case 0x4709:
case 0x470a:
case 0x470b:
case 0x470c:
case 0x470d:
case 0x470e:
case 0x470f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4710:
case 0x4711:
case 0x4712:
case 0x4713:
case 0x4714:
case 0x4715:
case 0x4716:
case 0x4717:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4718:
case 0x4719:
case 0x471a:
case 0x471b:
case 0x471c:
case 0x471d:
case 0x471e:
case 0x471f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4720:
case 0x4721:
case 0x4722:
case 0x4723:
case 0x4724:
case 0x4725:
case 0x4726:
case 0x4727:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4728:
case 0x4729:
case 0x472a:
case 0x472b:
case 0x472c:
case 0x472d:
case 0x472e:
case 0x472f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4730:
case 0x4731:
case 0x4732:
case 0x4733:
case 0x4734:
case 0x4735:
case 0x4736:
case 0x4737:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4738:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4739:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4740:
case 0x4741:
case 0x4742:
case 0x4743:
case 0x4744:
case 0x4745:
case 0x4746:
case 0x4747:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4748:
case 0x4749:
case 0x474a:
case 0x474b:
case 0x474c:
case 0x474d:
case 0x474e:
case 0x474f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4750:
case 0x4751:
case 0x4752:
case 0x4753:
case 0x4754:
case 0x4755:
case 0x4756:
case 0x4757:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4758:
case 0x4759:
case 0x475a:
case 0x475b:
case 0x475c:
case 0x475d:
case 0x475e:
case 0x475f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4760:
case 0x4761:
case 0x4762:
case 0x4763:
case 0x4764:
case 0x4765:
case 0x4766:
case 0x4767:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4768:
case 0x4769:
case 0x476a:
case 0x476b:
case 0x476c:
case 0x476d:
case 0x476e:
case 0x476f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4770:
case 0x4771:
case 0x4772:
case 0x4773:
case 0x4774:
case 0x4775:
case 0x4776:
case 0x4777:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4778:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4779:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4788:
case 0x4789:
case 0x478a:
case 0x478b:
case 0x478c:
case 0x478d:
case 0x478e:
case 0x478f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4790:
case 0x4791:
case 0x4792:
case 0x4793:
case 0x4794:
case 0x4795:
case 0x4796:
case 0x4797:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4798:
case 0x4799:
case 0x479a:
case 0x479b:
case 0x479c:
case 0x479d:
case 0x479e:
case 0x479f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x47a0:
case 0x47a1:
case 0x47a2:
case 0x47a3:
case 0x47a4:
case 0x47a5:
case 0x47a6:
case 0x47a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x47a8:
case 0x47a9:
case 0x47aa:
case 0x47ab:
case 0x47ac:
case 0x47ad:
case 0x47ae:
case 0x47af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x47b0:
case 0x47b1:
case 0x47b2:
case 0x47b3:
case 0x47b4:
case 0x47b5:
case 0x47b6:
case 0x47b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x47b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x47b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x47c0:
case 0x47c1:
case 0x47c2:
case 0x47c3:
case 0x47c4:
case 0x47c5:
case 0x47c6:
case 0x47c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47d0:
case 0x47d1:
case 0x47d2:
case 0x47d3:
case 0x47d4:
case 0x47d5:
case 0x47d6:
case 0x47d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47d8:
case 0x47d9:
case 0x47da:
case 0x47db:
case 0x47dc:
case 0x47dd:
case 0x47de:
case 0x47df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47e0:
case 0x47e1:
case 0x47e2:
case 0x47e3:
case 0x47e4:
case 0x47e5:
case 0x47e6:
case 0x47e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47e8:
case 0x47e9:
case 0x47ea:
case 0x47eb:
case 0x47ec:
case 0x47ed:
case 0x47ee:
case 0x47ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47f0:
case 0x47f1:
case 0x47f2:
case 0x47f3:
case 0x47f4:
case 0x47f5:
case 0x47f6:
case 0x47f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x47f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x47f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x47fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x47fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x47fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4800:
case 0x4801:
case 0x4802:
case 0x4803:
case 0x4804:
case 0x4805:
case 0x4806:
case 0x4807:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4810:
case 0x4811:
case 0x4812:
case 0x4813:
case 0x4814:
case 0x4815:
case 0x4816:
case 0x4817:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4818:
case 0x4819:
case 0x481a:
case 0x481b:
case 0x481c:
case 0x481d:
case 0x481e:
case 0x481f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4820:
case 0x4821:
case 0x4822:
case 0x4823:
case 0x4824:
case 0x4825:
case 0x4826:
case 0x4827:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4828:
case 0x4829:
case 0x482a:
case 0x482b:
case 0x482c:
case 0x482d:
case 0x482e:
case 0x482f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4830:
case 0x4831:
case 0x4832:
case 0x4833:
case 0x4834:
case 0x4835:
case 0x4836:
case 0x4837:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4838:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4839:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x483a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x483b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x483c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4840:
case 0x4841:
case 0x4842:
case 0x4843:
case 0x4844:
case 0x4845:
case 0x4846:
case 0x4847:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4850:
case 0x4851:
case 0x4852:
case 0x4853:
case 0x4854:
case 0x4855:
case 0x4856:
case 0x4857:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4858:
case 0x4859:
case 0x485a:
case 0x485b:
case 0x485c:
case 0x485d:
case 0x485e:
case 0x485f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4860:
case 0x4861:
case 0x4862:
case 0x4863:
case 0x4864:
case 0x4865:
case 0x4866:
case 0x4867:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4868:
case 0x4869:
case 0x486a:
case 0x486b:
case 0x486c:
case 0x486d:
case 0x486e:
case 0x486f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4870:
case 0x4871:
case 0x4872:
case 0x4873:
case 0x4874:
case 0x4875:
case 0x4876:
case 0x4877:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4878:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4879:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x487a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x487b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x487c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4880:
case 0x4881:
case 0x4882:
case 0x4883:
case 0x4884:
case 0x4885:
case 0x4886:
case 0x4887:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4890:
case 0x4891:
case 0x4892:
case 0x4893:
case 0x4894:
case 0x4895:
case 0x4896:
case 0x4897:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4898:
case 0x4899:
case 0x489a:
case 0x489b:
case 0x489c:
case 0x489d:
case 0x489e:
case 0x489f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48a0:
case 0x48a1:
case 0x48a2:
case 0x48a3:
case 0x48a4:
case 0x48a5:
case 0x48a6:
case 0x48a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48a8:
case 0x48a9:
case 0x48aa:
case 0x48ab:
case 0x48ac:
case 0x48ad:
case 0x48ae:
case 0x48af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48b0:
case 0x48b1:
case 0x48b2:
case 0x48b3:
case 0x48b4:
case 0x48b5:
case 0x48b6:
case 0x48b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x48c0:
case 0x48c1:
case 0x48c2:
case 0x48c3:
case 0x48c4:
case 0x48c5:
case 0x48c6:
case 0x48c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48d0:
case 0x48d1:
case 0x48d2:
case 0x48d3:
case 0x48d4:
case 0x48d5:
case 0x48d6:
case 0x48d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48d8:
case 0x48d9:
case 0x48da:
case 0x48db:
case 0x48dc:
case 0x48dd:
case 0x48de:
case 0x48df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48e0:
case 0x48e1:
case 0x48e2:
case 0x48e3:
case 0x48e4:
case 0x48e5:
case 0x48e6:
case 0x48e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48e8:
case 0x48e9:
case 0x48ea:
case 0x48eb:
case 0x48ec:
case 0x48ed:
case 0x48ee:
case 0x48ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48f0:
case 0x48f1:
case 0x48f2:
case 0x48f3:
case 0x48f4:
case 0x48f5:
case 0x48f6:
case 0x48f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x48f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x48f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x48fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x48fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x48fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4900:
case 0x4901:
case 0x4902:
case 0x4903:
case 0x4904:
case 0x4905:
case 0x4906:
case 0x4907:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4908:
case 0x4909:
case 0x490a:
case 0x490b:
case 0x490c:
case 0x490d:
case 0x490e:
case 0x490f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4910:
case 0x4911:
case 0x4912:
case 0x4913:
case 0x4914:
case 0x4915:
case 0x4916:
case 0x4917:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4918:
case 0x4919:
case 0x491a:
case 0x491b:
case 0x491c:
case 0x491d:
case 0x491e:
case 0x491f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4920:
case 0x4921:
case 0x4922:
case 0x4923:
case 0x4924:
case 0x4925:
case 0x4926:
case 0x4927:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4928:
case 0x4929:
case 0x492a:
case 0x492b:
case 0x492c:
case 0x492d:
case 0x492e:
case 0x492f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4930:
case 0x4931:
case 0x4932:
case 0x4933:
case 0x4934:
case 0x4935:
case 0x4936:
case 0x4937:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4938:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4939:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4940:
case 0x4941:
case 0x4942:
case 0x4943:
case 0x4944:
case 0x4945:
case 0x4946:
case 0x4947:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4948:
case 0x4949:
case 0x494a:
case 0x494b:
case 0x494c:
case 0x494d:
case 0x494e:
case 0x494f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4950:
case 0x4951:
case 0x4952:
case 0x4953:
case 0x4954:
case 0x4955:
case 0x4956:
case 0x4957:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4958:
case 0x4959:
case 0x495a:
case 0x495b:
case 0x495c:
case 0x495d:
case 0x495e:
case 0x495f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4960:
case 0x4961:
case 0x4962:
case 0x4963:
case 0x4964:
case 0x4965:
case 0x4966:
case 0x4967:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4968:
case 0x4969:
case 0x496a:
case 0x496b:
case 0x496c:
case 0x496d:
case 0x496e:
case 0x496f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4970:
case 0x4971:
case 0x4972:
case 0x4973:
case 0x4974:
case 0x4975:
case 0x4976:
case 0x4977:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4978:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4979:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4988:
case 0x4989:
case 0x498a:
case 0x498b:
case 0x498c:
case 0x498d:
case 0x498e:
case 0x498f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4990:
case 0x4991:
case 0x4992:
case 0x4993:
case 0x4994:
case 0x4995:
case 0x4996:
case 0x4997:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4998:
case 0x4999:
case 0x499a:
case 0x499b:
case 0x499c:
case 0x499d:
case 0x499e:
case 0x499f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x49a0:
case 0x49a1:
case 0x49a2:
case 0x49a3:
case 0x49a4:
case 0x49a5:
case 0x49a6:
case 0x49a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x49a8:
case 0x49a9:
case 0x49aa:
case 0x49ab:
case 0x49ac:
case 0x49ad:
case 0x49ae:
case 0x49af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x49b0:
case 0x49b1:
case 0x49b2:
case 0x49b3:
case 0x49b4:
case 0x49b5:
case 0x49b6:
case 0x49b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x49b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x49b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x49c0:
case 0x49c1:
case 0x49c2:
case 0x49c3:
case 0x49c4:
case 0x49c5:
case 0x49c6:
case 0x49c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49d0:
case 0x49d1:
case 0x49d2:
case 0x49d3:
case 0x49d4:
case 0x49d5:
case 0x49d6:
case 0x49d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49d8:
case 0x49d9:
case 0x49da:
case 0x49db:
case 0x49dc:
case 0x49dd:
case 0x49de:
case 0x49df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49e0:
case 0x49e1:
case 0x49e2:
case 0x49e3:
case 0x49e4:
case 0x49e5:
case 0x49e6:
case 0x49e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49e8:
case 0x49e9:
case 0x49ea:
case 0x49eb:
case 0x49ec:
case 0x49ed:
case 0x49ee:
case 0x49ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49f0:
case 0x49f1:
case 0x49f2:
case 0x49f3:
case 0x49f4:
case 0x49f5:
case 0x49f6:
case 0x49f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x49f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x49f9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x49fa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x49fb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x49fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4a00:
case 0x4a01:
case 0x4a02:
case 0x4a03:
case 0x4a04:
case 0x4a05:
case 0x4a06:
case 0x4a07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a10:
case 0x4a11:
case 0x4a12:
case 0x4a13:
case 0x4a14:
case 0x4a15:
case 0x4a16:
case 0x4a17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a18:
case 0x4a19:
case 0x4a1a:
case 0x4a1b:
case 0x4a1c:
case 0x4a1d:
case 0x4a1e:
case 0x4a1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a20:
case 0x4a21:
case 0x4a22:
case 0x4a23:
case 0x4a24:
case 0x4a25:
case 0x4a26:
case 0x4a27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a28:
case 0x4a29:
case 0x4a2a:
case 0x4a2b:
case 0x4a2c:
case 0x4a2d:
case 0x4a2e:
case 0x4a2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a30:
case 0x4a31:
case 0x4a32:
case 0x4a33:
case 0x4a34:
case 0x4a35:
case 0x4a36:
case 0x4a37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a40:
case 0x4a41:
case 0x4a42:
case 0x4a43:
case 0x4a44:
case 0x4a45:
case 0x4a46:
case 0x4a47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a50:
case 0x4a51:
case 0x4a52:
case 0x4a53:
case 0x4a54:
case 0x4a55:
case 0x4a56:
case 0x4a57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a58:
case 0x4a59:
case 0x4a5a:
case 0x4a5b:
case 0x4a5c:
case 0x4a5d:
case 0x4a5e:
case 0x4a5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a60:
case 0x4a61:
case 0x4a62:
case 0x4a63:
case 0x4a64:
case 0x4a65:
case 0x4a66:
case 0x4a67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a68:
case 0x4a69:
case 0x4a6a:
case 0x4a6b:
case 0x4a6c:
case 0x4a6d:
case 0x4a6e:
case 0x4a6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a70:
case 0x4a71:
case 0x4a72:
case 0x4a73:
case 0x4a74:
case 0x4a75:
case 0x4a76:
case 0x4a77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a80:
case 0x4a81:
case 0x4a82:
case 0x4a83:
case 0x4a84:
case 0x4a85:
case 0x4a86:
case 0x4a87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a90:
case 0x4a91:
case 0x4a92:
case 0x4a93:
case 0x4a94:
case 0x4a95:
case 0x4a96:
case 0x4a97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4a98:
case 0x4a99:
case 0x4a9a:
case 0x4a9b:
case 0x4a9c:
case 0x4a9d:
case 0x4a9e:
case 0x4a9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4aa0:
case 0x4aa1:
case 0x4aa2:
case 0x4aa3:
case 0x4aa4:
case 0x4aa5:
case 0x4aa6:
case 0x4aa7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4aa8:
case 0x4aa9:
case 0x4aaa:
case 0x4aab:
case 0x4aac:
case 0x4aad:
case 0x4aae:
case 0x4aaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ab0:
case 0x4ab1:
case 0x4ab2:
case 0x4ab3:
case 0x4ab4:
case 0x4ab5:
case 0x4ab6:
case 0x4ab7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ab8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ab9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4aba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4abb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4abc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ac0:
case 0x4ac1:
case 0x4ac2:
case 0x4ac3:
case 0x4ac4:
case 0x4ac5:
case 0x4ac6:
case 0x4ac7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ad0:
case 0x4ad1:
case 0x4ad2:
case 0x4ad3:
case 0x4ad4:
case 0x4ad5:
case 0x4ad6:
case 0x4ad7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ad8:
case 0x4ad9:
case 0x4ada:
case 0x4adb:
case 0x4adc:
case 0x4add:
case 0x4ade:
case 0x4adf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ae0:
case 0x4ae1:
case 0x4ae2:
case 0x4ae3:
case 0x4ae4:
case 0x4ae5:
case 0x4ae6:
case 0x4ae7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ae8:
case 0x4ae9:
case 0x4aea:
case 0x4aeb:
case 0x4aec:
case 0x4aed:
case 0x4aee:
case 0x4aef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4af0:
case 0x4af1:
case 0x4af2:
case 0x4af3:
case 0x4af4:
case 0x4af5:
case 0x4af6:
case 0x4af7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4af8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4af9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4afa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4afb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4afc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4b00:
case 0x4b01:
case 0x4b02:
case 0x4b03:
case 0x4b04:
case 0x4b05:
case 0x4b06:
case 0x4b07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4b08:
case 0x4b09:
case 0x4b0a:
case 0x4b0b:
case 0x4b0c:
case 0x4b0d:
case 0x4b0e:
case 0x4b0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4b10:
case 0x4b11:
case 0x4b12:
case 0x4b13:
case 0x4b14:
case 0x4b15:
case 0x4b16:
case 0x4b17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b18:
case 0x4b19:
case 0x4b1a:
case 0x4b1b:
case 0x4b1c:
case 0x4b1d:
case 0x4b1e:
case 0x4b1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b20:
case 0x4b21:
case 0x4b22:
case 0x4b23:
case 0x4b24:
case 0x4b25:
case 0x4b26:
case 0x4b27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b28:
case 0x4b29:
case 0x4b2a:
case 0x4b2b:
case 0x4b2c:
case 0x4b2d:
case 0x4b2e:
case 0x4b2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b30:
case 0x4b31:
case 0x4b32:
case 0x4b33:
case 0x4b34:
case 0x4b35:
case 0x4b36:
case 0x4b37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4b39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4b40:
case 0x4b41:
case 0x4b42:
case 0x4b43:
case 0x4b44:
case 0x4b45:
case 0x4b46:
case 0x4b47:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4b48:
case 0x4b49:
case 0x4b4a:
case 0x4b4b:
case 0x4b4c:
case 0x4b4d:
case 0x4b4e:
case 0x4b4f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4b50:
case 0x4b51:
case 0x4b52:
case 0x4b53:
case 0x4b54:
case 0x4b55:
case 0x4b56:
case 0x4b57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b58:
case 0x4b59:
case 0x4b5a:
case 0x4b5b:
case 0x4b5c:
case 0x4b5d:
case 0x4b5e:
case 0x4b5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b60:
case 0x4b61:
case 0x4b62:
case 0x4b63:
case 0x4b64:
case 0x4b65:
case 0x4b66:
case 0x4b67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b68:
case 0x4b69:
case 0x4b6a:
case 0x4b6b:
case 0x4b6c:
case 0x4b6d:
case 0x4b6e:
case 0x4b6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b70:
case 0x4b71:
case 0x4b72:
case 0x4b73:
case 0x4b74:
case 0x4b75:
case 0x4b76:
case 0x4b77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4b79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4b88:
case 0x4b89:
case 0x4b8a:
case 0x4b8b:
case 0x4b8c:
case 0x4b8d:
case 0x4b8e:
case 0x4b8f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4b90:
case 0x4b91:
case 0x4b92:
case 0x4b93:
case 0x4b94:
case 0x4b95:
case 0x4b96:
case 0x4b97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4b98:
case 0x4b99:
case 0x4b9a:
case 0x4b9b:
case 0x4b9c:
case 0x4b9d:
case 0x4b9e:
case 0x4b9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4ba0:
case 0x4ba1:
case 0x4ba2:
case 0x4ba3:
case 0x4ba4:
case 0x4ba5:
case 0x4ba6:
case 0x4ba7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4ba8:
case 0x4ba9:
case 0x4baa:
case 0x4bab:
case 0x4bac:
case 0x4bad:
case 0x4bae:
case 0x4baf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4bb0:
case 0x4bb1:
case 0x4bb2:
case 0x4bb3:
case 0x4bb4:
case 0x4bb5:
case 0x4bb6:
case 0x4bb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4bb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4bb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4bc0:
case 0x4bc1:
case 0x4bc2:
case 0x4bc3:
case 0x4bc4:
case 0x4bc5:
case 0x4bc6:
case 0x4bc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bd0:
case 0x4bd1:
case 0x4bd2:
case 0x4bd3:
case 0x4bd4:
case 0x4bd5:
case 0x4bd6:
case 0x4bd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bd8:
case 0x4bd9:
case 0x4bda:
case 0x4bdb:
case 0x4bdc:
case 0x4bdd:
case 0x4bde:
case 0x4bdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4be0:
case 0x4be1:
case 0x4be2:
case 0x4be3:
case 0x4be4:
case 0x4be5:
case 0x4be6:
case 0x4be7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4be8:
case 0x4be9:
case 0x4bea:
case 0x4beb:
case 0x4bec:
case 0x4bed:
case 0x4bee:
case 0x4bef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bf0:
case 0x4bf1:
case 0x4bf2:
case 0x4bf3:
case 0x4bf4:
case 0x4bf5:
case 0x4bf6:
case 0x4bf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bf9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bfa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bfb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4bfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4c00:
case 0x4c01:
case 0x4c02:
case 0x4c03:
case 0x4c04:
case 0x4c05:
case 0x4c06:
case 0x4c07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c10:
case 0x4c11:
case 0x4c12:
case 0x4c13:
case 0x4c14:
case 0x4c15:
case 0x4c16:
case 0x4c17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c18:
case 0x4c19:
case 0x4c1a:
case 0x4c1b:
case 0x4c1c:
case 0x4c1d:
case 0x4c1e:
case 0x4c1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c20:
case 0x4c21:
case 0x4c22:
case 0x4c23:
case 0x4c24:
case 0x4c25:
case 0x4c26:
case 0x4c27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c28:
case 0x4c29:
case 0x4c2a:
case 0x4c2b:
case 0x4c2c:
case 0x4c2d:
case 0x4c2e:
case 0x4c2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c30:
case 0x4c31:
case 0x4c32:
case 0x4c33:
case 0x4c34:
case 0x4c35:
case 0x4c36:
case 0x4c37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c40:
case 0x4c41:
case 0x4c42:
case 0x4c43:
case 0x4c44:
case 0x4c45:
case 0x4c46:
case 0x4c47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c50:
case 0x4c51:
case 0x4c52:
case 0x4c53:
case 0x4c54:
case 0x4c55:
case 0x4c56:
case 0x4c57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c58:
case 0x4c59:
case 0x4c5a:
case 0x4c5b:
case 0x4c5c:
case 0x4c5d:
case 0x4c5e:
case 0x4c5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c60:
case 0x4c61:
case 0x4c62:
case 0x4c63:
case 0x4c64:
case 0x4c65:
case 0x4c66:
case 0x4c67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c68:
case 0x4c69:
case 0x4c6a:
case 0x4c6b:
case 0x4c6c:
case 0x4c6d:
case 0x4c6e:
case 0x4c6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c70:
case 0x4c71:
case 0x4c72:
case 0x4c73:
case 0x4c74:
case 0x4c75:
case 0x4c76:
case 0x4c77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c80:
case 0x4c81:
case 0x4c82:
case 0x4c83:
case 0x4c84:
case 0x4c85:
case 0x4c86:
case 0x4c87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c90:
case 0x4c91:
case 0x4c92:
case 0x4c93:
case 0x4c94:
case 0x4c95:
case 0x4c96:
case 0x4c97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4c98:
case 0x4c99:
case 0x4c9a:
case 0x4c9b:
case 0x4c9c:
case 0x4c9d:
case 0x4c9e:
case 0x4c9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ca0:
case 0x4ca1:
case 0x4ca2:
case 0x4ca3:
case 0x4ca4:
case 0x4ca5:
case 0x4ca6:
case 0x4ca7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ca8:
case 0x4ca9:
case 0x4caa:
case 0x4cab:
case 0x4cac:
case 0x4cad:
case 0x4cae:
case 0x4caf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cb0:
case 0x4cb1:
case 0x4cb2:
case 0x4cb3:
case 0x4cb4:
case 0x4cb5:
case 0x4cb6:
case 0x4cb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cbb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cbc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4cc0:
case 0x4cc1:
case 0x4cc2:
case 0x4cc3:
case 0x4cc4:
case 0x4cc5:
case 0x4cc6:
case 0x4cc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cd0:
case 0x4cd1:
case 0x4cd2:
case 0x4cd3:
case 0x4cd4:
case 0x4cd5:
case 0x4cd6:
case 0x4cd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cd8:
case 0x4cd9:
case 0x4cda:
case 0x4cdb:
case 0x4cdc:
case 0x4cdd:
case 0x4cde:
case 0x4cdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ce0:
case 0x4ce1:
case 0x4ce2:
case 0x4ce3:
case 0x4ce4:
case 0x4ce5:
case 0x4ce6:
case 0x4ce7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ce8:
case 0x4ce9:
case 0x4cea:
case 0x4ceb:
case 0x4cec:
case 0x4ced:
case 0x4cee:
case 0x4cef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cf0:
case 0x4cf1:
case 0x4cf2:
case 0x4cf3:
case 0x4cf4:
case 0x4cf5:
case 0x4cf6:
case 0x4cf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cf9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cfa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cfb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4cfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4d00:
case 0x4d01:
case 0x4d02:
case 0x4d03:
case 0x4d04:
case 0x4d05:
case 0x4d06:
case 0x4d07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4d08:
case 0x4d09:
case 0x4d0a:
case 0x4d0b:
case 0x4d0c:
case 0x4d0d:
case 0x4d0e:
case 0x4d0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4d10:
case 0x4d11:
case 0x4d12:
case 0x4d13:
case 0x4d14:
case 0x4d15:
case 0x4d16:
case 0x4d17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d18:
case 0x4d19:
case 0x4d1a:
case 0x4d1b:
case 0x4d1c:
case 0x4d1d:
case 0x4d1e:
case 0x4d1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d20:
case 0x4d21:
case 0x4d22:
case 0x4d23:
case 0x4d24:
case 0x4d25:
case 0x4d26:
case 0x4d27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d28:
case 0x4d29:
case 0x4d2a:
case 0x4d2b:
case 0x4d2c:
case 0x4d2d:
case 0x4d2e:
case 0x4d2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d30:
case 0x4d31:
case 0x4d32:
case 0x4d33:
case 0x4d34:
case 0x4d35:
case 0x4d36:
case 0x4d37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4d39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4d40:
case 0x4d41:
case 0x4d42:
case 0x4d43:
case 0x4d44:
case 0x4d45:
case 0x4d46:
case 0x4d47:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4d48:
case 0x4d49:
case 0x4d4a:
case 0x4d4b:
case 0x4d4c:
case 0x4d4d:
case 0x4d4e:
case 0x4d4f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4d50:
case 0x4d51:
case 0x4d52:
case 0x4d53:
case 0x4d54:
case 0x4d55:
case 0x4d56:
case 0x4d57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d58:
case 0x4d59:
case 0x4d5a:
case 0x4d5b:
case 0x4d5c:
case 0x4d5d:
case 0x4d5e:
case 0x4d5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d60:
case 0x4d61:
case 0x4d62:
case 0x4d63:
case 0x4d64:
case 0x4d65:
case 0x4d66:
case 0x4d67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d68:
case 0x4d69:
case 0x4d6a:
case 0x4d6b:
case 0x4d6c:
case 0x4d6d:
case 0x4d6e:
case 0x4d6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d70:
case 0x4d71:
case 0x4d72:
case 0x4d73:
case 0x4d74:
case 0x4d75:
case 0x4d76:
case 0x4d77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4d79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4d88:
case 0x4d89:
case 0x4d8a:
case 0x4d8b:
case 0x4d8c:
case 0x4d8d:
case 0x4d8e:
case 0x4d8f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4d90:
case 0x4d91:
case 0x4d92:
case 0x4d93:
case 0x4d94:
case 0x4d95:
case 0x4d96:
case 0x4d97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4d98:
case 0x4d99:
case 0x4d9a:
case 0x4d9b:
case 0x4d9c:
case 0x4d9d:
case 0x4d9e:
case 0x4d9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4da0:
case 0x4da1:
case 0x4da2:
case 0x4da3:
case 0x4da4:
case 0x4da5:
case 0x4da6:
case 0x4da7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4da8:
case 0x4da9:
case 0x4daa:
case 0x4dab:
case 0x4dac:
case 0x4dad:
case 0x4dae:
case 0x4daf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4db0:
case 0x4db1:
case 0x4db2:
case 0x4db3:
case 0x4db4:
case 0x4db5:
case 0x4db6:
case 0x4db7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4db8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4db9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4dc0:
case 0x4dc1:
case 0x4dc2:
case 0x4dc3:
case 0x4dc4:
case 0x4dc5:
case 0x4dc6:
case 0x4dc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4dd0:
case 0x4dd1:
case 0x4dd2:
case 0x4dd3:
case 0x4dd4:
case 0x4dd5:
case 0x4dd6:
case 0x4dd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4dd8:
case 0x4dd9:
case 0x4dda:
case 0x4ddb:
case 0x4ddc:
case 0x4ddd:
case 0x4dde:
case 0x4ddf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4de0:
case 0x4de1:
case 0x4de2:
case 0x4de3:
case 0x4de4:
case 0x4de5:
case 0x4de6:
case 0x4de7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4de8:
case 0x4de9:
case 0x4dea:
case 0x4deb:
case 0x4dec:
case 0x4ded:
case 0x4dee:
case 0x4def:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4df0:
case 0x4df1:
case 0x4df2:
case 0x4df3:
case 0x4df4:
case 0x4df5:
case 0x4df6:
case 0x4df7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4df8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4df9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4dfa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4dfb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4dfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4e00:
case 0x4e01:
case 0x4e02:
case 0x4e03:
case 0x4e04:
case 0x4e05:
case 0x4e06:
case 0x4e07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e10:
case 0x4e11:
case 0x4e12:
case 0x4e13:
case 0x4e14:
case 0x4e15:
case 0x4e16:
case 0x4e17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e18:
case 0x4e19:
case 0x4e1a:
case 0x4e1b:
case 0x4e1c:
case 0x4e1d:
case 0x4e1e:
case 0x4e1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e20:
case 0x4e21:
case 0x4e22:
case 0x4e23:
case 0x4e24:
case 0x4e25:
case 0x4e26:
case 0x4e27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e28:
case 0x4e29:
case 0x4e2a:
case 0x4e2b:
case 0x4e2c:
case 0x4e2d:
case 0x4e2e:
case 0x4e2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e30:
case 0x4e31:
case 0x4e32:
case 0x4e33:
case 0x4e34:
case 0x4e35:
case 0x4e36:
case 0x4e37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e40:
case 0x4e41:
case 0x4e42:
case 0x4e43:
case 0x4e44:
case 0x4e45:
case 0x4e46:
case 0x4e47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e50:
case 0x4e51:
case 0x4e52:
case 0x4e53:
case 0x4e54:
case 0x4e55:
case 0x4e56:
case 0x4e57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e58:
case 0x4e59:
case 0x4e5a:
case 0x4e5b:
case 0x4e5c:
case 0x4e5d:
case 0x4e5e:
case 0x4e5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e60:
case 0x4e61:
case 0x4e62:
case 0x4e63:
case 0x4e64:
case 0x4e65:
case 0x4e66:
case 0x4e67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e68:
case 0x4e69:
case 0x4e6a:
case 0x4e6b:
case 0x4e6c:
case 0x4e6d:
case 0x4e6e:
case 0x4e6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e70:
case 0x4e71:
case 0x4e72:
case 0x4e73:
case 0x4e74:
case 0x4e75:
case 0x4e76:
case 0x4e77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e80:
case 0x4e81:
case 0x4e82:
case 0x4e83:
case 0x4e84:
case 0x4e85:
case 0x4e86:
case 0x4e87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e90:
case 0x4e91:
case 0x4e92:
case 0x4e93:
case 0x4e94:
case 0x4e95:
case 0x4e96:
case 0x4e97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4e98:
case 0x4e99:
case 0x4e9a:
case 0x4e9b:
case 0x4e9c:
case 0x4e9d:
case 0x4e9e:
case 0x4e9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ea0:
case 0x4ea1:
case 0x4ea2:
case 0x4ea3:
case 0x4ea4:
case 0x4ea5:
case 0x4ea6:
case 0x4ea7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ea8:
case 0x4ea9:
case 0x4eaa:
case 0x4eab:
case 0x4eac:
case 0x4ead:
case 0x4eae:
case 0x4eaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4eb0:
case 0x4eb1:
case 0x4eb2:
case 0x4eb3:
case 0x4eb4:
case 0x4eb5:
case 0x4eb6:
case 0x4eb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4eb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4eb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4eba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ebb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ebc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); AND(src, dst);
	}
	break;

case 0x4ec0:
case 0x4ec1:
case 0x4ec2:
case 0x4ec3:
case 0x4ec4:
case 0x4ec5:
case 0x4ec6:
case 0x4ec7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ed0:
case 0x4ed1:
case 0x4ed2:
case 0x4ed3:
case 0x4ed4:
case 0x4ed5:
case 0x4ed6:
case 0x4ed7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ed8:
case 0x4ed9:
case 0x4eda:
case 0x4edb:
case 0x4edc:
case 0x4edd:
case 0x4ede:
case 0x4edf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ee0:
case 0x4ee1:
case 0x4ee2:
case 0x4ee3:
case 0x4ee4:
case 0x4ee5:
case 0x4ee6:
case 0x4ee7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ee8:
case 0x4ee9:
case 0x4eea:
case 0x4eeb:
case 0x4eec:
case 0x4eed:
case 0x4eee:
case 0x4eef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ef0:
case 0x4ef1:
case 0x4ef2:
case 0x4ef3:
case 0x4ef4:
case 0x4ef5:
case 0x4ef6:
case 0x4ef7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ef8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4ef9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4efa:
	{
	 HAM<uint16, PC_DISP> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4efb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4efc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULU(src, instr_b11_b9);
	}
	break;

case 0x4f00:
case 0x4f01:
case 0x4f02:
case 0x4f03:
case 0x4f04:
case 0x4f05:
case 0x4f06:
case 0x4f07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4f08:
case 0x4f09:
case 0x4f0a:
case 0x4f0b:
case 0x4f0c:
case 0x4f0d:
case 0x4f0e:
case 0x4f0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ABCD(src, dst);
	}
	break;

case 0x4f10:
case 0x4f11:
case 0x4f12:
case 0x4f13:
case 0x4f14:
case 0x4f15:
case 0x4f16:
case 0x4f17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f18:
case 0x4f19:
case 0x4f1a:
case 0x4f1b:
case 0x4f1c:
case 0x4f1d:
case 0x4f1e:
case 0x4f1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f20:
case 0x4f21:
case 0x4f22:
case 0x4f23:
case 0x4f24:
case 0x4f25:
case 0x4f26:
case 0x4f27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f28:
case 0x4f29:
case 0x4f2a:
case 0x4f2b:
case 0x4f2c:
case 0x4f2d:
case 0x4f2e:
case 0x4f2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f30:
case 0x4f31:
case 0x4f32:
case 0x4f33:
case 0x4f34:
case 0x4f35:
case 0x4f36:
case 0x4f37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4f39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4f40:
case 0x4f41:
case 0x4f42:
case 0x4f43:
case 0x4f44:
case 0x4f45:
case 0x4f46:
case 0x4f47:
	{
	 EXG(&D[instr_b11_b9], &D[instr_b2_b0]); /* EXG Dx, Dy */
	}
	break;

case 0x4f48:
case 0x4f49:
case 0x4f4a:
case 0x4f4b:
case 0x4f4c:
case 0x4f4d:
case 0x4f4e:
case 0x4f4f:
	{
	 EXG(&A[instr_b11_b9], &A[instr_b2_b0]); /* EXG Ax, Ay */
	}
	break;

case 0x4f50:
case 0x4f51:
case 0x4f52:
case 0x4f53:
case 0x4f54:
case 0x4f55:
case 0x4f56:
case 0x4f57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f58:
case 0x4f59:
case 0x4f5a:
case 0x4f5b:
case 0x4f5c:
case 0x4f5d:
case 0x4f5e:
case 0x4f5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f60:
case 0x4f61:
case 0x4f62:
case 0x4f63:
case 0x4f64:
case 0x4f65:
case 0x4f66:
case 0x4f67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f68:
case 0x4f69:
case 0x4f6a:
case 0x4f6b:
case 0x4f6c:
case 0x4f6d:
case 0x4f6e:
case 0x4f6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f70:
case 0x4f71:
case 0x4f72:
case 0x4f73:
case 0x4f74:
case 0x4f75:
case 0x4f76:
case 0x4f77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4f79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4f88:
case 0x4f89:
case 0x4f8a:
case 0x4f8b:
case 0x4f8c:
case 0x4f8d:
case 0x4f8e:
case 0x4f8f:
	{
	 EXG(&D[instr_b11_b9], &A[instr_b2_b0]); /* EXG Dx, Ay */
	}
	break;

case 0x4f90:
case 0x4f91:
case 0x4f92:
case 0x4f93:
case 0x4f94:
case 0x4f95:
case 0x4f96:
case 0x4f97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4f98:
case 0x4f99:
case 0x4f9a:
case 0x4f9b:
case 0x4f9c:
case 0x4f9d:
case 0x4f9e:
case 0x4f9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4fa0:
case 0x4fa1:
case 0x4fa2:
case 0x4fa3:
case 0x4fa4:
case 0x4fa5:
case 0x4fa6:
case 0x4fa7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4fa8:
case 0x4fa9:
case 0x4faa:
case 0x4fab:
case 0x4fac:
case 0x4fad:
case 0x4fae:
case 0x4faf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4fb0:
case 0x4fb1:
case 0x4fb2:
case 0x4fb3:
case 0x4fb4:
case 0x4fb5:
case 0x4fb6:
case 0x4fb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); AND(src, dst);
	}
	break;

case 0x4fb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); AND(src, dst);
	}
	break;

case 0x4fb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); AND(src, dst);
	}
	break;

case 0x4fc0:
case 0x4fc1:
case 0x4fc2:
case 0x4fc3:
case 0x4fc4:
case 0x4fc5:
case 0x4fc6:
case 0x4fc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4fd0:
case 0x4fd1:
case 0x4fd2:
case 0x4fd3:
case 0x4fd4:
case 0x4fd5:
case 0x4fd6:
case 0x4fd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4fd8:
case 0x4fd9:
case 0x4fda:
case 0x4fdb:
case 0x4fdc:
case 0x4fdd:
case 0x4fde:
case 0x4fdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4fe0:
case 0x4fe1:
case 0x4fe2:
case 0x4fe3:
case 0x4fe4:
case 0x4fe5:
case 0x4fe6:
case 0x4fe7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4fe8:
case 0x4fe9:
case 0x4fea:
case 0x4feb:
case 0x4fec:
case 0x4fed:
case 0x4fee:
case 0x4fef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ff0:
case 0x4ff1:
case 0x4ff2:
case 0x4ff3:
case 0x4ff4:
case 0x4ff5:
case 0x4ff6:
case 0x4ff7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ff8:
	{
	 HAM<uint16, ABS_SHORT> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ff9:
	{
	 HAM<uint16, ABS_LONG> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ffa:
	{
	 HAM<uint16, PC_DISP> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ffb:
	{
	 HAM<uint16, PC_INDEX> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x4ffc:
	{
	 HAM<uint16, IMMEDIATE> src(this); MULS(src, instr_b11_b9);
	}
	break;

case 0x5000:
case 0x5001:
case 0x5002:
case 0x5003:
case 0x5004:
case 0x5005:
case 0x5006:
case 0x5007:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5010:
case 0x5011:
case 0x5012:
case 0x5013:
case 0x5014:
case 0x5015:
case 0x5016:
case 0x5017:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5018:
case 0x5019:
case 0x501a:
case 0x501b:
case 0x501c:
case 0x501d:
case 0x501e:
case 0x501f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5020:
case 0x5021:
case 0x5022:
case 0x5023:
case 0x5024:
case 0x5025:
case 0x5026:
case 0x5027:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5028:
case 0x5029:
case 0x502a:
case 0x502b:
case 0x502c:
case 0x502d:
case 0x502e:
case 0x502f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5030:
case 0x5031:
case 0x5032:
case 0x5033:
case 0x5034:
case 0x5035:
case 0x5036:
case 0x5037:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5038:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5039:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x503a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x503b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x503c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5040:
case 0x5041:
case 0x5042:
case 0x5043:
case 0x5044:
case 0x5045:
case 0x5046:
case 0x5047:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5048:
case 0x5049:
case 0x504a:
case 0x504b:
case 0x504c:
case 0x504d:
case 0x504e:
case 0x504f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5050:
case 0x5051:
case 0x5052:
case 0x5053:
case 0x5054:
case 0x5055:
case 0x5056:
case 0x5057:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5058:
case 0x5059:
case 0x505a:
case 0x505b:
case 0x505c:
case 0x505d:
case 0x505e:
case 0x505f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5060:
case 0x5061:
case 0x5062:
case 0x5063:
case 0x5064:
case 0x5065:
case 0x5066:
case 0x5067:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5068:
case 0x5069:
case 0x506a:
case 0x506b:
case 0x506c:
case 0x506d:
case 0x506e:
case 0x506f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5070:
case 0x5071:
case 0x5072:
case 0x5073:
case 0x5074:
case 0x5075:
case 0x5076:
case 0x5077:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5078:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5079:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x507a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x507b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x507c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5080:
case 0x5081:
case 0x5082:
case 0x5083:
case 0x5084:
case 0x5085:
case 0x5086:
case 0x5087:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5088:
case 0x5089:
case 0x508a:
case 0x508b:
case 0x508c:
case 0x508d:
case 0x508e:
case 0x508f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5090:
case 0x5091:
case 0x5092:
case 0x5093:
case 0x5094:
case 0x5095:
case 0x5096:
case 0x5097:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5098:
case 0x5099:
case 0x509a:
case 0x509b:
case 0x509c:
case 0x509d:
case 0x509e:
case 0x509f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50a0:
case 0x50a1:
case 0x50a2:
case 0x50a3:
case 0x50a4:
case 0x50a5:
case 0x50a6:
case 0x50a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50a8:
case 0x50a9:
case 0x50aa:
case 0x50ab:
case 0x50ac:
case 0x50ad:
case 0x50ae:
case 0x50af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50b0:
case 0x50b1:
case 0x50b2:
case 0x50b3:
case 0x50b4:
case 0x50b5:
case 0x50b6:
case 0x50b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50c0:
case 0x50c1:
case 0x50c2:
case 0x50c3:
case 0x50c4:
case 0x50c5:
case 0x50c6:
case 0x50c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50c8:
case 0x50c9:
case 0x50ca:
case 0x50cb:
case 0x50cc:
case 0x50cd:
case 0x50ce:
case 0x50cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50d0:
case 0x50d1:
case 0x50d2:
case 0x50d3:
case 0x50d4:
case 0x50d5:
case 0x50d6:
case 0x50d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50d8:
case 0x50d9:
case 0x50da:
case 0x50db:
case 0x50dc:
case 0x50dd:
case 0x50de:
case 0x50df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50e0:
case 0x50e1:
case 0x50e2:
case 0x50e3:
case 0x50e4:
case 0x50e5:
case 0x50e6:
case 0x50e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50e8:
case 0x50e9:
case 0x50ea:
case 0x50eb:
case 0x50ec:
case 0x50ed:
case 0x50ee:
case 0x50ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50f0:
case 0x50f1:
case 0x50f2:
case 0x50f3:
case 0x50f4:
case 0x50f5:
case 0x50f6:
case 0x50f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x50fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5100:
case 0x5101:
case 0x5102:
case 0x5103:
case 0x5104:
case 0x5105:
case 0x5106:
case 0x5107:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5108:
case 0x5109:
case 0x510a:
case 0x510b:
case 0x510c:
case 0x510d:
case 0x510e:
case 0x510f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5110:
case 0x5111:
case 0x5112:
case 0x5113:
case 0x5114:
case 0x5115:
case 0x5116:
case 0x5117:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5118:
case 0x5119:
case 0x511a:
case 0x511b:
case 0x511c:
case 0x511d:
case 0x511e:
case 0x511f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5120:
case 0x5121:
case 0x5122:
case 0x5123:
case 0x5124:
case 0x5125:
case 0x5126:
case 0x5127:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5128:
case 0x5129:
case 0x512a:
case 0x512b:
case 0x512c:
case 0x512d:
case 0x512e:
case 0x512f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5130:
case 0x5131:
case 0x5132:
case 0x5133:
case 0x5134:
case 0x5135:
case 0x5136:
case 0x5137:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5138:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5139:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5140:
case 0x5141:
case 0x5142:
case 0x5143:
case 0x5144:
case 0x5145:
case 0x5146:
case 0x5147:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5148:
case 0x5149:
case 0x514a:
case 0x514b:
case 0x514c:
case 0x514d:
case 0x514e:
case 0x514f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5150:
case 0x5151:
case 0x5152:
case 0x5153:
case 0x5154:
case 0x5155:
case 0x5156:
case 0x5157:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5158:
case 0x5159:
case 0x515a:
case 0x515b:
case 0x515c:
case 0x515d:
case 0x515e:
case 0x515f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5160:
case 0x5161:
case 0x5162:
case 0x5163:
case 0x5164:
case 0x5165:
case 0x5166:
case 0x5167:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5168:
case 0x5169:
case 0x516a:
case 0x516b:
case 0x516c:
case 0x516d:
case 0x516e:
case 0x516f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5170:
case 0x5171:
case 0x5172:
case 0x5173:
case 0x5174:
case 0x5175:
case 0x5176:
case 0x5177:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5178:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5179:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5180:
case 0x5181:
case 0x5182:
case 0x5183:
case 0x5184:
case 0x5185:
case 0x5186:
case 0x5187:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5188:
case 0x5189:
case 0x518a:
case 0x518b:
case 0x518c:
case 0x518d:
case 0x518e:
case 0x518f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5190:
case 0x5191:
case 0x5192:
case 0x5193:
case 0x5194:
case 0x5195:
case 0x5196:
case 0x5197:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5198:
case 0x5199:
case 0x519a:
case 0x519b:
case 0x519c:
case 0x519d:
case 0x519e:
case 0x519f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x51a0:
case 0x51a1:
case 0x51a2:
case 0x51a3:
case 0x51a4:
case 0x51a5:
case 0x51a6:
case 0x51a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x51a8:
case 0x51a9:
case 0x51aa:
case 0x51ab:
case 0x51ac:
case 0x51ad:
case 0x51ae:
case 0x51af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x51b0:
case 0x51b1:
case 0x51b2:
case 0x51b3:
case 0x51b4:
case 0x51b5:
case 0x51b6:
case 0x51b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x51b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x51b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x51c0:
case 0x51c1:
case 0x51c2:
case 0x51c3:
case 0x51c4:
case 0x51c5:
case 0x51c6:
case 0x51c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51c8:
case 0x51c9:
case 0x51ca:
case 0x51cb:
case 0x51cc:
case 0x51cd:
case 0x51ce:
case 0x51cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51d0:
case 0x51d1:
case 0x51d2:
case 0x51d3:
case 0x51d4:
case 0x51d5:
case 0x51d6:
case 0x51d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51d8:
case 0x51d9:
case 0x51da:
case 0x51db:
case 0x51dc:
case 0x51dd:
case 0x51de:
case 0x51df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51e0:
case 0x51e1:
case 0x51e2:
case 0x51e3:
case 0x51e4:
case 0x51e5:
case 0x51e6:
case 0x51e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51e8:
case 0x51e9:
case 0x51ea:
case 0x51eb:
case 0x51ec:
case 0x51ed:
case 0x51ee:
case 0x51ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51f0:
case 0x51f1:
case 0x51f2:
case 0x51f3:
case 0x51f4:
case 0x51f5:
case 0x51f6:
case 0x51f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x51fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5200:
case 0x5201:
case 0x5202:
case 0x5203:
case 0x5204:
case 0x5205:
case 0x5206:
case 0x5207:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5210:
case 0x5211:
case 0x5212:
case 0x5213:
case 0x5214:
case 0x5215:
case 0x5216:
case 0x5217:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5218:
case 0x5219:
case 0x521a:
case 0x521b:
case 0x521c:
case 0x521d:
case 0x521e:
case 0x521f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5220:
case 0x5221:
case 0x5222:
case 0x5223:
case 0x5224:
case 0x5225:
case 0x5226:
case 0x5227:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5228:
case 0x5229:
case 0x522a:
case 0x522b:
case 0x522c:
case 0x522d:
case 0x522e:
case 0x522f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5230:
case 0x5231:
case 0x5232:
case 0x5233:
case 0x5234:
case 0x5235:
case 0x5236:
case 0x5237:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5238:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5239:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x523a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x523b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x523c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5240:
case 0x5241:
case 0x5242:
case 0x5243:
case 0x5244:
case 0x5245:
case 0x5246:
case 0x5247:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5248:
case 0x5249:
case 0x524a:
case 0x524b:
case 0x524c:
case 0x524d:
case 0x524e:
case 0x524f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5250:
case 0x5251:
case 0x5252:
case 0x5253:
case 0x5254:
case 0x5255:
case 0x5256:
case 0x5257:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5258:
case 0x5259:
case 0x525a:
case 0x525b:
case 0x525c:
case 0x525d:
case 0x525e:
case 0x525f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5260:
case 0x5261:
case 0x5262:
case 0x5263:
case 0x5264:
case 0x5265:
case 0x5266:
case 0x5267:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5268:
case 0x5269:
case 0x526a:
case 0x526b:
case 0x526c:
case 0x526d:
case 0x526e:
case 0x526f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5270:
case 0x5271:
case 0x5272:
case 0x5273:
case 0x5274:
case 0x5275:
case 0x5276:
case 0x5277:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5278:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5279:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x527a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x527b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x527c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5280:
case 0x5281:
case 0x5282:
case 0x5283:
case 0x5284:
case 0x5285:
case 0x5286:
case 0x5287:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5288:
case 0x5289:
case 0x528a:
case 0x528b:
case 0x528c:
case 0x528d:
case 0x528e:
case 0x528f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5290:
case 0x5291:
case 0x5292:
case 0x5293:
case 0x5294:
case 0x5295:
case 0x5296:
case 0x5297:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5298:
case 0x5299:
case 0x529a:
case 0x529b:
case 0x529c:
case 0x529d:
case 0x529e:
case 0x529f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52a0:
case 0x52a1:
case 0x52a2:
case 0x52a3:
case 0x52a4:
case 0x52a5:
case 0x52a6:
case 0x52a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52a8:
case 0x52a9:
case 0x52aa:
case 0x52ab:
case 0x52ac:
case 0x52ad:
case 0x52ae:
case 0x52af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52b0:
case 0x52b1:
case 0x52b2:
case 0x52b3:
case 0x52b4:
case 0x52b5:
case 0x52b6:
case 0x52b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52c0:
case 0x52c1:
case 0x52c2:
case 0x52c3:
case 0x52c4:
case 0x52c5:
case 0x52c6:
case 0x52c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52c8:
case 0x52c9:
case 0x52ca:
case 0x52cb:
case 0x52cc:
case 0x52cd:
case 0x52ce:
case 0x52cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52d0:
case 0x52d1:
case 0x52d2:
case 0x52d3:
case 0x52d4:
case 0x52d5:
case 0x52d6:
case 0x52d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52d8:
case 0x52d9:
case 0x52da:
case 0x52db:
case 0x52dc:
case 0x52dd:
case 0x52de:
case 0x52df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52e0:
case 0x52e1:
case 0x52e2:
case 0x52e3:
case 0x52e4:
case 0x52e5:
case 0x52e6:
case 0x52e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52e8:
case 0x52e9:
case 0x52ea:
case 0x52eb:
case 0x52ec:
case 0x52ed:
case 0x52ee:
case 0x52ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52f0:
case 0x52f1:
case 0x52f2:
case 0x52f3:
case 0x52f4:
case 0x52f5:
case 0x52f6:
case 0x52f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x52fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5300:
case 0x5301:
case 0x5302:
case 0x5303:
case 0x5304:
case 0x5305:
case 0x5306:
case 0x5307:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5308:
case 0x5309:
case 0x530a:
case 0x530b:
case 0x530c:
case 0x530d:
case 0x530e:
case 0x530f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5310:
case 0x5311:
case 0x5312:
case 0x5313:
case 0x5314:
case 0x5315:
case 0x5316:
case 0x5317:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5318:
case 0x5319:
case 0x531a:
case 0x531b:
case 0x531c:
case 0x531d:
case 0x531e:
case 0x531f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5320:
case 0x5321:
case 0x5322:
case 0x5323:
case 0x5324:
case 0x5325:
case 0x5326:
case 0x5327:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5328:
case 0x5329:
case 0x532a:
case 0x532b:
case 0x532c:
case 0x532d:
case 0x532e:
case 0x532f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5330:
case 0x5331:
case 0x5332:
case 0x5333:
case 0x5334:
case 0x5335:
case 0x5336:
case 0x5337:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5338:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5339:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5340:
case 0x5341:
case 0x5342:
case 0x5343:
case 0x5344:
case 0x5345:
case 0x5346:
case 0x5347:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5348:
case 0x5349:
case 0x534a:
case 0x534b:
case 0x534c:
case 0x534d:
case 0x534e:
case 0x534f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5350:
case 0x5351:
case 0x5352:
case 0x5353:
case 0x5354:
case 0x5355:
case 0x5356:
case 0x5357:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5358:
case 0x5359:
case 0x535a:
case 0x535b:
case 0x535c:
case 0x535d:
case 0x535e:
case 0x535f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5360:
case 0x5361:
case 0x5362:
case 0x5363:
case 0x5364:
case 0x5365:
case 0x5366:
case 0x5367:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5368:
case 0x5369:
case 0x536a:
case 0x536b:
case 0x536c:
case 0x536d:
case 0x536e:
case 0x536f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5370:
case 0x5371:
case 0x5372:
case 0x5373:
case 0x5374:
case 0x5375:
case 0x5376:
case 0x5377:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5378:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5379:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5380:
case 0x5381:
case 0x5382:
case 0x5383:
case 0x5384:
case 0x5385:
case 0x5386:
case 0x5387:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5388:
case 0x5389:
case 0x538a:
case 0x538b:
case 0x538c:
case 0x538d:
case 0x538e:
case 0x538f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5390:
case 0x5391:
case 0x5392:
case 0x5393:
case 0x5394:
case 0x5395:
case 0x5396:
case 0x5397:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5398:
case 0x5399:
case 0x539a:
case 0x539b:
case 0x539c:
case 0x539d:
case 0x539e:
case 0x539f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x53a0:
case 0x53a1:
case 0x53a2:
case 0x53a3:
case 0x53a4:
case 0x53a5:
case 0x53a6:
case 0x53a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x53a8:
case 0x53a9:
case 0x53aa:
case 0x53ab:
case 0x53ac:
case 0x53ad:
case 0x53ae:
case 0x53af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x53b0:
case 0x53b1:
case 0x53b2:
case 0x53b3:
case 0x53b4:
case 0x53b5:
case 0x53b6:
case 0x53b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x53b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x53b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x53c0:
case 0x53c1:
case 0x53c2:
case 0x53c3:
case 0x53c4:
case 0x53c5:
case 0x53c6:
case 0x53c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53c8:
case 0x53c9:
case 0x53ca:
case 0x53cb:
case 0x53cc:
case 0x53cd:
case 0x53ce:
case 0x53cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53d0:
case 0x53d1:
case 0x53d2:
case 0x53d3:
case 0x53d4:
case 0x53d5:
case 0x53d6:
case 0x53d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53d8:
case 0x53d9:
case 0x53da:
case 0x53db:
case 0x53dc:
case 0x53dd:
case 0x53de:
case 0x53df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53e0:
case 0x53e1:
case 0x53e2:
case 0x53e3:
case 0x53e4:
case 0x53e5:
case 0x53e6:
case 0x53e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53e8:
case 0x53e9:
case 0x53ea:
case 0x53eb:
case 0x53ec:
case 0x53ed:
case 0x53ee:
case 0x53ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53f0:
case 0x53f1:
case 0x53f2:
case 0x53f3:
case 0x53f4:
case 0x53f5:
case 0x53f6:
case 0x53f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x53fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5400:
case 0x5401:
case 0x5402:
case 0x5403:
case 0x5404:
case 0x5405:
case 0x5406:
case 0x5407:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5410:
case 0x5411:
case 0x5412:
case 0x5413:
case 0x5414:
case 0x5415:
case 0x5416:
case 0x5417:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5418:
case 0x5419:
case 0x541a:
case 0x541b:
case 0x541c:
case 0x541d:
case 0x541e:
case 0x541f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5420:
case 0x5421:
case 0x5422:
case 0x5423:
case 0x5424:
case 0x5425:
case 0x5426:
case 0x5427:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5428:
case 0x5429:
case 0x542a:
case 0x542b:
case 0x542c:
case 0x542d:
case 0x542e:
case 0x542f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5430:
case 0x5431:
case 0x5432:
case 0x5433:
case 0x5434:
case 0x5435:
case 0x5436:
case 0x5437:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5438:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5439:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x543a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x543b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x543c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5440:
case 0x5441:
case 0x5442:
case 0x5443:
case 0x5444:
case 0x5445:
case 0x5446:
case 0x5447:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5448:
case 0x5449:
case 0x544a:
case 0x544b:
case 0x544c:
case 0x544d:
case 0x544e:
case 0x544f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5450:
case 0x5451:
case 0x5452:
case 0x5453:
case 0x5454:
case 0x5455:
case 0x5456:
case 0x5457:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5458:
case 0x5459:
case 0x545a:
case 0x545b:
case 0x545c:
case 0x545d:
case 0x545e:
case 0x545f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5460:
case 0x5461:
case 0x5462:
case 0x5463:
case 0x5464:
case 0x5465:
case 0x5466:
case 0x5467:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5468:
case 0x5469:
case 0x546a:
case 0x546b:
case 0x546c:
case 0x546d:
case 0x546e:
case 0x546f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5470:
case 0x5471:
case 0x5472:
case 0x5473:
case 0x5474:
case 0x5475:
case 0x5476:
case 0x5477:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5478:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5479:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x547a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x547b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x547c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5480:
case 0x5481:
case 0x5482:
case 0x5483:
case 0x5484:
case 0x5485:
case 0x5486:
case 0x5487:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5488:
case 0x5489:
case 0x548a:
case 0x548b:
case 0x548c:
case 0x548d:
case 0x548e:
case 0x548f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5490:
case 0x5491:
case 0x5492:
case 0x5493:
case 0x5494:
case 0x5495:
case 0x5496:
case 0x5497:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5498:
case 0x5499:
case 0x549a:
case 0x549b:
case 0x549c:
case 0x549d:
case 0x549e:
case 0x549f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54a0:
case 0x54a1:
case 0x54a2:
case 0x54a3:
case 0x54a4:
case 0x54a5:
case 0x54a6:
case 0x54a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54a8:
case 0x54a9:
case 0x54aa:
case 0x54ab:
case 0x54ac:
case 0x54ad:
case 0x54ae:
case 0x54af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54b0:
case 0x54b1:
case 0x54b2:
case 0x54b3:
case 0x54b4:
case 0x54b5:
case 0x54b6:
case 0x54b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54c0:
case 0x54c1:
case 0x54c2:
case 0x54c3:
case 0x54c4:
case 0x54c5:
case 0x54c6:
case 0x54c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54c8:
case 0x54c9:
case 0x54ca:
case 0x54cb:
case 0x54cc:
case 0x54cd:
case 0x54ce:
case 0x54cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54d0:
case 0x54d1:
case 0x54d2:
case 0x54d3:
case 0x54d4:
case 0x54d5:
case 0x54d6:
case 0x54d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54d8:
case 0x54d9:
case 0x54da:
case 0x54db:
case 0x54dc:
case 0x54dd:
case 0x54de:
case 0x54df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54e0:
case 0x54e1:
case 0x54e2:
case 0x54e3:
case 0x54e4:
case 0x54e5:
case 0x54e6:
case 0x54e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54e8:
case 0x54e9:
case 0x54ea:
case 0x54eb:
case 0x54ec:
case 0x54ed:
case 0x54ee:
case 0x54ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54f0:
case 0x54f1:
case 0x54f2:
case 0x54f3:
case 0x54f4:
case 0x54f5:
case 0x54f6:
case 0x54f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x54fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5500:
case 0x5501:
case 0x5502:
case 0x5503:
case 0x5504:
case 0x5505:
case 0x5506:
case 0x5507:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5508:
case 0x5509:
case 0x550a:
case 0x550b:
case 0x550c:
case 0x550d:
case 0x550e:
case 0x550f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5510:
case 0x5511:
case 0x5512:
case 0x5513:
case 0x5514:
case 0x5515:
case 0x5516:
case 0x5517:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5518:
case 0x5519:
case 0x551a:
case 0x551b:
case 0x551c:
case 0x551d:
case 0x551e:
case 0x551f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5520:
case 0x5521:
case 0x5522:
case 0x5523:
case 0x5524:
case 0x5525:
case 0x5526:
case 0x5527:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5528:
case 0x5529:
case 0x552a:
case 0x552b:
case 0x552c:
case 0x552d:
case 0x552e:
case 0x552f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5530:
case 0x5531:
case 0x5532:
case 0x5533:
case 0x5534:
case 0x5535:
case 0x5536:
case 0x5537:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5538:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5539:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5540:
case 0x5541:
case 0x5542:
case 0x5543:
case 0x5544:
case 0x5545:
case 0x5546:
case 0x5547:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5548:
case 0x5549:
case 0x554a:
case 0x554b:
case 0x554c:
case 0x554d:
case 0x554e:
case 0x554f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5550:
case 0x5551:
case 0x5552:
case 0x5553:
case 0x5554:
case 0x5555:
case 0x5556:
case 0x5557:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5558:
case 0x5559:
case 0x555a:
case 0x555b:
case 0x555c:
case 0x555d:
case 0x555e:
case 0x555f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5560:
case 0x5561:
case 0x5562:
case 0x5563:
case 0x5564:
case 0x5565:
case 0x5566:
case 0x5567:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5568:
case 0x5569:
case 0x556a:
case 0x556b:
case 0x556c:
case 0x556d:
case 0x556e:
case 0x556f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5570:
case 0x5571:
case 0x5572:
case 0x5573:
case 0x5574:
case 0x5575:
case 0x5576:
case 0x5577:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5578:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5579:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5580:
case 0x5581:
case 0x5582:
case 0x5583:
case 0x5584:
case 0x5585:
case 0x5586:
case 0x5587:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5588:
case 0x5589:
case 0x558a:
case 0x558b:
case 0x558c:
case 0x558d:
case 0x558e:
case 0x558f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5590:
case 0x5591:
case 0x5592:
case 0x5593:
case 0x5594:
case 0x5595:
case 0x5596:
case 0x5597:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5598:
case 0x5599:
case 0x559a:
case 0x559b:
case 0x559c:
case 0x559d:
case 0x559e:
case 0x559f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x55a0:
case 0x55a1:
case 0x55a2:
case 0x55a3:
case 0x55a4:
case 0x55a5:
case 0x55a6:
case 0x55a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x55a8:
case 0x55a9:
case 0x55aa:
case 0x55ab:
case 0x55ac:
case 0x55ad:
case 0x55ae:
case 0x55af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x55b0:
case 0x55b1:
case 0x55b2:
case 0x55b3:
case 0x55b4:
case 0x55b5:
case 0x55b6:
case 0x55b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x55b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x55b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x55c0:
case 0x55c1:
case 0x55c2:
case 0x55c3:
case 0x55c4:
case 0x55c5:
case 0x55c6:
case 0x55c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55c8:
case 0x55c9:
case 0x55ca:
case 0x55cb:
case 0x55cc:
case 0x55cd:
case 0x55ce:
case 0x55cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55d0:
case 0x55d1:
case 0x55d2:
case 0x55d3:
case 0x55d4:
case 0x55d5:
case 0x55d6:
case 0x55d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55d8:
case 0x55d9:
case 0x55da:
case 0x55db:
case 0x55dc:
case 0x55dd:
case 0x55de:
case 0x55df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55e0:
case 0x55e1:
case 0x55e2:
case 0x55e3:
case 0x55e4:
case 0x55e5:
case 0x55e6:
case 0x55e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55e8:
case 0x55e9:
case 0x55ea:
case 0x55eb:
case 0x55ec:
case 0x55ed:
case 0x55ee:
case 0x55ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55f0:
case 0x55f1:
case 0x55f2:
case 0x55f3:
case 0x55f4:
case 0x55f5:
case 0x55f6:
case 0x55f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x55fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5600:
case 0x5601:
case 0x5602:
case 0x5603:
case 0x5604:
case 0x5605:
case 0x5606:
case 0x5607:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5610:
case 0x5611:
case 0x5612:
case 0x5613:
case 0x5614:
case 0x5615:
case 0x5616:
case 0x5617:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5618:
case 0x5619:
case 0x561a:
case 0x561b:
case 0x561c:
case 0x561d:
case 0x561e:
case 0x561f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5620:
case 0x5621:
case 0x5622:
case 0x5623:
case 0x5624:
case 0x5625:
case 0x5626:
case 0x5627:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5628:
case 0x5629:
case 0x562a:
case 0x562b:
case 0x562c:
case 0x562d:
case 0x562e:
case 0x562f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5630:
case 0x5631:
case 0x5632:
case 0x5633:
case 0x5634:
case 0x5635:
case 0x5636:
case 0x5637:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5638:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5639:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x563a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x563b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x563c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5640:
case 0x5641:
case 0x5642:
case 0x5643:
case 0x5644:
case 0x5645:
case 0x5646:
case 0x5647:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5648:
case 0x5649:
case 0x564a:
case 0x564b:
case 0x564c:
case 0x564d:
case 0x564e:
case 0x564f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5650:
case 0x5651:
case 0x5652:
case 0x5653:
case 0x5654:
case 0x5655:
case 0x5656:
case 0x5657:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5658:
case 0x5659:
case 0x565a:
case 0x565b:
case 0x565c:
case 0x565d:
case 0x565e:
case 0x565f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5660:
case 0x5661:
case 0x5662:
case 0x5663:
case 0x5664:
case 0x5665:
case 0x5666:
case 0x5667:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5668:
case 0x5669:
case 0x566a:
case 0x566b:
case 0x566c:
case 0x566d:
case 0x566e:
case 0x566f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5670:
case 0x5671:
case 0x5672:
case 0x5673:
case 0x5674:
case 0x5675:
case 0x5676:
case 0x5677:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5678:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5679:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x567a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x567b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x567c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5680:
case 0x5681:
case 0x5682:
case 0x5683:
case 0x5684:
case 0x5685:
case 0x5686:
case 0x5687:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5688:
case 0x5689:
case 0x568a:
case 0x568b:
case 0x568c:
case 0x568d:
case 0x568e:
case 0x568f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5690:
case 0x5691:
case 0x5692:
case 0x5693:
case 0x5694:
case 0x5695:
case 0x5696:
case 0x5697:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5698:
case 0x5699:
case 0x569a:
case 0x569b:
case 0x569c:
case 0x569d:
case 0x569e:
case 0x569f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56a0:
case 0x56a1:
case 0x56a2:
case 0x56a3:
case 0x56a4:
case 0x56a5:
case 0x56a6:
case 0x56a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56a8:
case 0x56a9:
case 0x56aa:
case 0x56ab:
case 0x56ac:
case 0x56ad:
case 0x56ae:
case 0x56af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56b0:
case 0x56b1:
case 0x56b2:
case 0x56b3:
case 0x56b4:
case 0x56b5:
case 0x56b6:
case 0x56b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56c0:
case 0x56c1:
case 0x56c2:
case 0x56c3:
case 0x56c4:
case 0x56c5:
case 0x56c6:
case 0x56c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56c8:
case 0x56c9:
case 0x56ca:
case 0x56cb:
case 0x56cc:
case 0x56cd:
case 0x56ce:
case 0x56cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56d0:
case 0x56d1:
case 0x56d2:
case 0x56d3:
case 0x56d4:
case 0x56d5:
case 0x56d6:
case 0x56d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56d8:
case 0x56d9:
case 0x56da:
case 0x56db:
case 0x56dc:
case 0x56dd:
case 0x56de:
case 0x56df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56e0:
case 0x56e1:
case 0x56e2:
case 0x56e3:
case 0x56e4:
case 0x56e5:
case 0x56e6:
case 0x56e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56e8:
case 0x56e9:
case 0x56ea:
case 0x56eb:
case 0x56ec:
case 0x56ed:
case 0x56ee:
case 0x56ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56f0:
case 0x56f1:
case 0x56f2:
case 0x56f3:
case 0x56f4:
case 0x56f5:
case 0x56f6:
case 0x56f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x56fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5700:
case 0x5701:
case 0x5702:
case 0x5703:
case 0x5704:
case 0x5705:
case 0x5706:
case 0x5707:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5708:
case 0x5709:
case 0x570a:
case 0x570b:
case 0x570c:
case 0x570d:
case 0x570e:
case 0x570f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5710:
case 0x5711:
case 0x5712:
case 0x5713:
case 0x5714:
case 0x5715:
case 0x5716:
case 0x5717:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5718:
case 0x5719:
case 0x571a:
case 0x571b:
case 0x571c:
case 0x571d:
case 0x571e:
case 0x571f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5720:
case 0x5721:
case 0x5722:
case 0x5723:
case 0x5724:
case 0x5725:
case 0x5726:
case 0x5727:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5728:
case 0x5729:
case 0x572a:
case 0x572b:
case 0x572c:
case 0x572d:
case 0x572e:
case 0x572f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5730:
case 0x5731:
case 0x5732:
case 0x5733:
case 0x5734:
case 0x5735:
case 0x5736:
case 0x5737:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5738:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5739:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5740:
case 0x5741:
case 0x5742:
case 0x5743:
case 0x5744:
case 0x5745:
case 0x5746:
case 0x5747:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5748:
case 0x5749:
case 0x574a:
case 0x574b:
case 0x574c:
case 0x574d:
case 0x574e:
case 0x574f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5750:
case 0x5751:
case 0x5752:
case 0x5753:
case 0x5754:
case 0x5755:
case 0x5756:
case 0x5757:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5758:
case 0x5759:
case 0x575a:
case 0x575b:
case 0x575c:
case 0x575d:
case 0x575e:
case 0x575f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5760:
case 0x5761:
case 0x5762:
case 0x5763:
case 0x5764:
case 0x5765:
case 0x5766:
case 0x5767:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5768:
case 0x5769:
case 0x576a:
case 0x576b:
case 0x576c:
case 0x576d:
case 0x576e:
case 0x576f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5770:
case 0x5771:
case 0x5772:
case 0x5773:
case 0x5774:
case 0x5775:
case 0x5776:
case 0x5777:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5778:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5779:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5780:
case 0x5781:
case 0x5782:
case 0x5783:
case 0x5784:
case 0x5785:
case 0x5786:
case 0x5787:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5788:
case 0x5789:
case 0x578a:
case 0x578b:
case 0x578c:
case 0x578d:
case 0x578e:
case 0x578f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5790:
case 0x5791:
case 0x5792:
case 0x5793:
case 0x5794:
case 0x5795:
case 0x5796:
case 0x5797:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5798:
case 0x5799:
case 0x579a:
case 0x579b:
case 0x579c:
case 0x579d:
case 0x579e:
case 0x579f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x57a0:
case 0x57a1:
case 0x57a2:
case 0x57a3:
case 0x57a4:
case 0x57a5:
case 0x57a6:
case 0x57a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x57a8:
case 0x57a9:
case 0x57aa:
case 0x57ab:
case 0x57ac:
case 0x57ad:
case 0x57ae:
case 0x57af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x57b0:
case 0x57b1:
case 0x57b2:
case 0x57b3:
case 0x57b4:
case 0x57b5:
case 0x57b6:
case 0x57b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x57b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x57b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x57c0:
case 0x57c1:
case 0x57c2:
case 0x57c3:
case 0x57c4:
case 0x57c5:
case 0x57c6:
case 0x57c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57c8:
case 0x57c9:
case 0x57ca:
case 0x57cb:
case 0x57cc:
case 0x57cd:
case 0x57ce:
case 0x57cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57d0:
case 0x57d1:
case 0x57d2:
case 0x57d3:
case 0x57d4:
case 0x57d5:
case 0x57d6:
case 0x57d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57d8:
case 0x57d9:
case 0x57da:
case 0x57db:
case 0x57dc:
case 0x57dd:
case 0x57de:
case 0x57df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57e0:
case 0x57e1:
case 0x57e2:
case 0x57e3:
case 0x57e4:
case 0x57e5:
case 0x57e6:
case 0x57e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57e8:
case 0x57e9:
case 0x57ea:
case 0x57eb:
case 0x57ec:
case 0x57ed:
case 0x57ee:
case 0x57ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57f0:
case 0x57f1:
case 0x57f2:
case 0x57f3:
case 0x57f4:
case 0x57f5:
case 0x57f6:
case 0x57f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x57fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5800:
case 0x5801:
case 0x5802:
case 0x5803:
case 0x5804:
case 0x5805:
case 0x5806:
case 0x5807:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5810:
case 0x5811:
case 0x5812:
case 0x5813:
case 0x5814:
case 0x5815:
case 0x5816:
case 0x5817:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5818:
case 0x5819:
case 0x581a:
case 0x581b:
case 0x581c:
case 0x581d:
case 0x581e:
case 0x581f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5820:
case 0x5821:
case 0x5822:
case 0x5823:
case 0x5824:
case 0x5825:
case 0x5826:
case 0x5827:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5828:
case 0x5829:
case 0x582a:
case 0x582b:
case 0x582c:
case 0x582d:
case 0x582e:
case 0x582f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5830:
case 0x5831:
case 0x5832:
case 0x5833:
case 0x5834:
case 0x5835:
case 0x5836:
case 0x5837:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5838:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5839:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x583a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x583b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x583c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5840:
case 0x5841:
case 0x5842:
case 0x5843:
case 0x5844:
case 0x5845:
case 0x5846:
case 0x5847:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5848:
case 0x5849:
case 0x584a:
case 0x584b:
case 0x584c:
case 0x584d:
case 0x584e:
case 0x584f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5850:
case 0x5851:
case 0x5852:
case 0x5853:
case 0x5854:
case 0x5855:
case 0x5856:
case 0x5857:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5858:
case 0x5859:
case 0x585a:
case 0x585b:
case 0x585c:
case 0x585d:
case 0x585e:
case 0x585f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5860:
case 0x5861:
case 0x5862:
case 0x5863:
case 0x5864:
case 0x5865:
case 0x5866:
case 0x5867:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5868:
case 0x5869:
case 0x586a:
case 0x586b:
case 0x586c:
case 0x586d:
case 0x586e:
case 0x586f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5870:
case 0x5871:
case 0x5872:
case 0x5873:
case 0x5874:
case 0x5875:
case 0x5876:
case 0x5877:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5878:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5879:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x587a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x587b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x587c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5880:
case 0x5881:
case 0x5882:
case 0x5883:
case 0x5884:
case 0x5885:
case 0x5886:
case 0x5887:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5888:
case 0x5889:
case 0x588a:
case 0x588b:
case 0x588c:
case 0x588d:
case 0x588e:
case 0x588f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5890:
case 0x5891:
case 0x5892:
case 0x5893:
case 0x5894:
case 0x5895:
case 0x5896:
case 0x5897:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5898:
case 0x5899:
case 0x589a:
case 0x589b:
case 0x589c:
case 0x589d:
case 0x589e:
case 0x589f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58a0:
case 0x58a1:
case 0x58a2:
case 0x58a3:
case 0x58a4:
case 0x58a5:
case 0x58a6:
case 0x58a7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58a8:
case 0x58a9:
case 0x58aa:
case 0x58ab:
case 0x58ac:
case 0x58ad:
case 0x58ae:
case 0x58af:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58b0:
case 0x58b1:
case 0x58b2:
case 0x58b3:
case 0x58b4:
case 0x58b5:
case 0x58b6:
case 0x58b7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58b8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58b9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58ba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58bb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58bc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58c0:
case 0x58c1:
case 0x58c2:
case 0x58c3:
case 0x58c4:
case 0x58c5:
case 0x58c6:
case 0x58c7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58c8:
case 0x58c9:
case 0x58ca:
case 0x58cb:
case 0x58cc:
case 0x58cd:
case 0x58ce:
case 0x58cf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58d0:
case 0x58d1:
case 0x58d2:
case 0x58d3:
case 0x58d4:
case 0x58d5:
case 0x58d6:
case 0x58d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58d8:
case 0x58d9:
case 0x58da:
case 0x58db:
case 0x58dc:
case 0x58dd:
case 0x58de:
case 0x58df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58e0:
case 0x58e1:
case 0x58e2:
case 0x58e3:
case 0x58e4:
case 0x58e5:
case 0x58e6:
case 0x58e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58e8:
case 0x58e9:
case 0x58ea:
case 0x58eb:
case 0x58ec:
case 0x58ed:
case 0x58ee:
case 0x58ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58f0:
case 0x58f1:
case 0x58f2:
case 0x58f3:
case 0x58f4:
case 0x58f5:
case 0x58f6:
case 0x58f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58f8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58f9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58fa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58fb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x58fc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5900:
case 0x5901:
case 0x5902:
case 0x5903:
case 0x5904:
case 0x5905:
case 0x5906:
case 0x5907:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5908:
case 0x5909:
case 0x590a:
case 0x590b:
case 0x590c:
case 0x590d:
case 0x590e:
case 0x590f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5910:
case 0x5911:
case 0x5912:
case 0x5913:
case 0x5914:
case 0x5915:
case 0x5916:
case 0x5917:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5918:
case 0x5919:
case 0x591a:
case 0x591b:
case 0x591c:
case 0x591d:
case 0x591e:
case 0x591f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5920:
case 0x5921:
case 0x5922:
case 0x5923:
case 0x5924:
case 0x5925:
case 0x5926:
case 0x5927:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5928:
case 0x5929:
case 0x592a:
case 0x592b:
case 0x592c:
case 0x592d:
case 0x592e:
case 0x592f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5930:
case 0x5931:
case 0x5932:
case 0x5933:
case 0x5934:
case 0x5935:
case 0x5936:
case 0x5937:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5938:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5939:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5940:
case 0x5941:
case 0x5942:
case 0x5943:
case 0x5944:
case 0x5945:
case 0x5946:
case 0x5947:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5948:
case 0x5949:
case 0x594a:
case 0x594b:
case 0x594c:
case 0x594d:
case 0x594e:
case 0x594f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5950:
case 0x5951:
case 0x5952:
case 0x5953:
case 0x5954:
case 0x5955:
case 0x5956:
case 0x5957:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5958:
case 0x5959:
case 0x595a:
case 0x595b:
case 0x595c:
case 0x595d:
case 0x595e:
case 0x595f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5960:
case 0x5961:
case 0x5962:
case 0x5963:
case 0x5964:
case 0x5965:
case 0x5966:
case 0x5967:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5968:
case 0x5969:
case 0x596a:
case 0x596b:
case 0x596c:
case 0x596d:
case 0x596e:
case 0x596f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5970:
case 0x5971:
case 0x5972:
case 0x5973:
case 0x5974:
case 0x5975:
case 0x5976:
case 0x5977:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5978:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5979:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5980:
case 0x5981:
case 0x5982:
case 0x5983:
case 0x5984:
case 0x5985:
case 0x5986:
case 0x5987:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5988:
case 0x5989:
case 0x598a:
case 0x598b:
case 0x598c:
case 0x598d:
case 0x598e:
case 0x598f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5990:
case 0x5991:
case 0x5992:
case 0x5993:
case 0x5994:
case 0x5995:
case 0x5996:
case 0x5997:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5998:
case 0x5999:
case 0x599a:
case 0x599b:
case 0x599c:
case 0x599d:
case 0x599e:
case 0x599f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x59a0:
case 0x59a1:
case 0x59a2:
case 0x59a3:
case 0x59a4:
case 0x59a5:
case 0x59a6:
case 0x59a7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x59a8:
case 0x59a9:
case 0x59aa:
case 0x59ab:
case 0x59ac:
case 0x59ad:
case 0x59ae:
case 0x59af:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x59b0:
case 0x59b1:
case 0x59b2:
case 0x59b3:
case 0x59b4:
case 0x59b5:
case 0x59b6:
case 0x59b7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x59b8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x59b9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x59c0:
case 0x59c1:
case 0x59c2:
case 0x59c3:
case 0x59c4:
case 0x59c5:
case 0x59c6:
case 0x59c7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59c8:
case 0x59c9:
case 0x59ca:
case 0x59cb:
case 0x59cc:
case 0x59cd:
case 0x59ce:
case 0x59cf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59d0:
case 0x59d1:
case 0x59d2:
case 0x59d3:
case 0x59d4:
case 0x59d5:
case 0x59d6:
case 0x59d7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59d8:
case 0x59d9:
case 0x59da:
case 0x59db:
case 0x59dc:
case 0x59dd:
case 0x59de:
case 0x59df:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59e0:
case 0x59e1:
case 0x59e2:
case 0x59e3:
case 0x59e4:
case 0x59e5:
case 0x59e6:
case 0x59e7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59e8:
case 0x59e9:
case 0x59ea:
case 0x59eb:
case 0x59ec:
case 0x59ed:
case 0x59ee:
case 0x59ef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59f0:
case 0x59f1:
case 0x59f2:
case 0x59f3:
case 0x59f4:
case 0x59f5:
case 0x59f6:
case 0x59f7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59f8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59f9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59fa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59fb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x59fc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a00:
case 0x5a01:
case 0x5a02:
case 0x5a03:
case 0x5a04:
case 0x5a05:
case 0x5a06:
case 0x5a07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a10:
case 0x5a11:
case 0x5a12:
case 0x5a13:
case 0x5a14:
case 0x5a15:
case 0x5a16:
case 0x5a17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a18:
case 0x5a19:
case 0x5a1a:
case 0x5a1b:
case 0x5a1c:
case 0x5a1d:
case 0x5a1e:
case 0x5a1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a20:
case 0x5a21:
case 0x5a22:
case 0x5a23:
case 0x5a24:
case 0x5a25:
case 0x5a26:
case 0x5a27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a28:
case 0x5a29:
case 0x5a2a:
case 0x5a2b:
case 0x5a2c:
case 0x5a2d:
case 0x5a2e:
case 0x5a2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a30:
case 0x5a31:
case 0x5a32:
case 0x5a33:
case 0x5a34:
case 0x5a35:
case 0x5a36:
case 0x5a37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a40:
case 0x5a41:
case 0x5a42:
case 0x5a43:
case 0x5a44:
case 0x5a45:
case 0x5a46:
case 0x5a47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a48:
case 0x5a49:
case 0x5a4a:
case 0x5a4b:
case 0x5a4c:
case 0x5a4d:
case 0x5a4e:
case 0x5a4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a50:
case 0x5a51:
case 0x5a52:
case 0x5a53:
case 0x5a54:
case 0x5a55:
case 0x5a56:
case 0x5a57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a58:
case 0x5a59:
case 0x5a5a:
case 0x5a5b:
case 0x5a5c:
case 0x5a5d:
case 0x5a5e:
case 0x5a5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a60:
case 0x5a61:
case 0x5a62:
case 0x5a63:
case 0x5a64:
case 0x5a65:
case 0x5a66:
case 0x5a67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a68:
case 0x5a69:
case 0x5a6a:
case 0x5a6b:
case 0x5a6c:
case 0x5a6d:
case 0x5a6e:
case 0x5a6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a70:
case 0x5a71:
case 0x5a72:
case 0x5a73:
case 0x5a74:
case 0x5a75:
case 0x5a76:
case 0x5a77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a80:
case 0x5a81:
case 0x5a82:
case 0x5a83:
case 0x5a84:
case 0x5a85:
case 0x5a86:
case 0x5a87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a88:
case 0x5a89:
case 0x5a8a:
case 0x5a8b:
case 0x5a8c:
case 0x5a8d:
case 0x5a8e:
case 0x5a8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a90:
case 0x5a91:
case 0x5a92:
case 0x5a93:
case 0x5a94:
case 0x5a95:
case 0x5a96:
case 0x5a97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5a98:
case 0x5a99:
case 0x5a9a:
case 0x5a9b:
case 0x5a9c:
case 0x5a9d:
case 0x5a9e:
case 0x5a9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5aa0:
case 0x5aa1:
case 0x5aa2:
case 0x5aa3:
case 0x5aa4:
case 0x5aa5:
case 0x5aa6:
case 0x5aa7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5aa8:
case 0x5aa9:
case 0x5aaa:
case 0x5aab:
case 0x5aac:
case 0x5aad:
case 0x5aae:
case 0x5aaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ab0:
case 0x5ab1:
case 0x5ab2:
case 0x5ab3:
case 0x5ab4:
case 0x5ab5:
case 0x5ab6:
case 0x5ab7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ab8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ab9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5aba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5abb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5abc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ac0:
case 0x5ac1:
case 0x5ac2:
case 0x5ac3:
case 0x5ac4:
case 0x5ac5:
case 0x5ac6:
case 0x5ac7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ac8:
case 0x5ac9:
case 0x5aca:
case 0x5acb:
case 0x5acc:
case 0x5acd:
case 0x5ace:
case 0x5acf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ad0:
case 0x5ad1:
case 0x5ad2:
case 0x5ad3:
case 0x5ad4:
case 0x5ad5:
case 0x5ad6:
case 0x5ad7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ad8:
case 0x5ad9:
case 0x5ada:
case 0x5adb:
case 0x5adc:
case 0x5add:
case 0x5ade:
case 0x5adf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ae0:
case 0x5ae1:
case 0x5ae2:
case 0x5ae3:
case 0x5ae4:
case 0x5ae5:
case 0x5ae6:
case 0x5ae7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ae8:
case 0x5ae9:
case 0x5aea:
case 0x5aeb:
case 0x5aec:
case 0x5aed:
case 0x5aee:
case 0x5aef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5af0:
case 0x5af1:
case 0x5af2:
case 0x5af3:
case 0x5af4:
case 0x5af5:
case 0x5af6:
case 0x5af7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5af8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5af9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5afa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5afb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5afc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5b00:
case 0x5b01:
case 0x5b02:
case 0x5b03:
case 0x5b04:
case 0x5b05:
case 0x5b06:
case 0x5b07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b08:
case 0x5b09:
case 0x5b0a:
case 0x5b0b:
case 0x5b0c:
case 0x5b0d:
case 0x5b0e:
case 0x5b0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b10:
case 0x5b11:
case 0x5b12:
case 0x5b13:
case 0x5b14:
case 0x5b15:
case 0x5b16:
case 0x5b17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b18:
case 0x5b19:
case 0x5b1a:
case 0x5b1b:
case 0x5b1c:
case 0x5b1d:
case 0x5b1e:
case 0x5b1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b20:
case 0x5b21:
case 0x5b22:
case 0x5b23:
case 0x5b24:
case 0x5b25:
case 0x5b26:
case 0x5b27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b28:
case 0x5b29:
case 0x5b2a:
case 0x5b2b:
case 0x5b2c:
case 0x5b2d:
case 0x5b2e:
case 0x5b2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b30:
case 0x5b31:
case 0x5b32:
case 0x5b33:
case 0x5b34:
case 0x5b35:
case 0x5b36:
case 0x5b37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5b39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5b40:
case 0x5b41:
case 0x5b42:
case 0x5b43:
case 0x5b44:
case 0x5b45:
case 0x5b46:
case 0x5b47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b48:
case 0x5b49:
case 0x5b4a:
case 0x5b4b:
case 0x5b4c:
case 0x5b4d:
case 0x5b4e:
case 0x5b4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b50:
case 0x5b51:
case 0x5b52:
case 0x5b53:
case 0x5b54:
case 0x5b55:
case 0x5b56:
case 0x5b57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b58:
case 0x5b59:
case 0x5b5a:
case 0x5b5b:
case 0x5b5c:
case 0x5b5d:
case 0x5b5e:
case 0x5b5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b60:
case 0x5b61:
case 0x5b62:
case 0x5b63:
case 0x5b64:
case 0x5b65:
case 0x5b66:
case 0x5b67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b68:
case 0x5b69:
case 0x5b6a:
case 0x5b6b:
case 0x5b6c:
case 0x5b6d:
case 0x5b6e:
case 0x5b6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b70:
case 0x5b71:
case 0x5b72:
case 0x5b73:
case 0x5b74:
case 0x5b75:
case 0x5b76:
case 0x5b77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5b79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5b80:
case 0x5b81:
case 0x5b82:
case 0x5b83:
case 0x5b84:
case 0x5b85:
case 0x5b86:
case 0x5b87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b88:
case 0x5b89:
case 0x5b8a:
case 0x5b8b:
case 0x5b8c:
case 0x5b8d:
case 0x5b8e:
case 0x5b8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5b90:
case 0x5b91:
case 0x5b92:
case 0x5b93:
case 0x5b94:
case 0x5b95:
case 0x5b96:
case 0x5b97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5b98:
case 0x5b99:
case 0x5b9a:
case 0x5b9b:
case 0x5b9c:
case 0x5b9d:
case 0x5b9e:
case 0x5b9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5ba0:
case 0x5ba1:
case 0x5ba2:
case 0x5ba3:
case 0x5ba4:
case 0x5ba5:
case 0x5ba6:
case 0x5ba7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5ba8:
case 0x5ba9:
case 0x5baa:
case 0x5bab:
case 0x5bac:
case 0x5bad:
case 0x5bae:
case 0x5baf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5bb0:
case 0x5bb1:
case 0x5bb2:
case 0x5bb3:
case 0x5bb4:
case 0x5bb5:
case 0x5bb6:
case 0x5bb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5bb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5bb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5bc0:
case 0x5bc1:
case 0x5bc2:
case 0x5bc3:
case 0x5bc4:
case 0x5bc5:
case 0x5bc6:
case 0x5bc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bc8:
case 0x5bc9:
case 0x5bca:
case 0x5bcb:
case 0x5bcc:
case 0x5bcd:
case 0x5bce:
case 0x5bcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bd0:
case 0x5bd1:
case 0x5bd2:
case 0x5bd3:
case 0x5bd4:
case 0x5bd5:
case 0x5bd6:
case 0x5bd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bd8:
case 0x5bd9:
case 0x5bda:
case 0x5bdb:
case 0x5bdc:
case 0x5bdd:
case 0x5bde:
case 0x5bdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5be0:
case 0x5be1:
case 0x5be2:
case 0x5be3:
case 0x5be4:
case 0x5be5:
case 0x5be6:
case 0x5be7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5be8:
case 0x5be9:
case 0x5bea:
case 0x5beb:
case 0x5bec:
case 0x5bed:
case 0x5bee:
case 0x5bef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bf0:
case 0x5bf1:
case 0x5bf2:
case 0x5bf3:
case 0x5bf4:
case 0x5bf5:
case 0x5bf6:
case 0x5bf7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bf8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bf9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5bfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c00:
case 0x5c01:
case 0x5c02:
case 0x5c03:
case 0x5c04:
case 0x5c05:
case 0x5c06:
case 0x5c07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c10:
case 0x5c11:
case 0x5c12:
case 0x5c13:
case 0x5c14:
case 0x5c15:
case 0x5c16:
case 0x5c17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c18:
case 0x5c19:
case 0x5c1a:
case 0x5c1b:
case 0x5c1c:
case 0x5c1d:
case 0x5c1e:
case 0x5c1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c20:
case 0x5c21:
case 0x5c22:
case 0x5c23:
case 0x5c24:
case 0x5c25:
case 0x5c26:
case 0x5c27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c28:
case 0x5c29:
case 0x5c2a:
case 0x5c2b:
case 0x5c2c:
case 0x5c2d:
case 0x5c2e:
case 0x5c2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c30:
case 0x5c31:
case 0x5c32:
case 0x5c33:
case 0x5c34:
case 0x5c35:
case 0x5c36:
case 0x5c37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c40:
case 0x5c41:
case 0x5c42:
case 0x5c43:
case 0x5c44:
case 0x5c45:
case 0x5c46:
case 0x5c47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c48:
case 0x5c49:
case 0x5c4a:
case 0x5c4b:
case 0x5c4c:
case 0x5c4d:
case 0x5c4e:
case 0x5c4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c50:
case 0x5c51:
case 0x5c52:
case 0x5c53:
case 0x5c54:
case 0x5c55:
case 0x5c56:
case 0x5c57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c58:
case 0x5c59:
case 0x5c5a:
case 0x5c5b:
case 0x5c5c:
case 0x5c5d:
case 0x5c5e:
case 0x5c5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c60:
case 0x5c61:
case 0x5c62:
case 0x5c63:
case 0x5c64:
case 0x5c65:
case 0x5c66:
case 0x5c67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c68:
case 0x5c69:
case 0x5c6a:
case 0x5c6b:
case 0x5c6c:
case 0x5c6d:
case 0x5c6e:
case 0x5c6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c70:
case 0x5c71:
case 0x5c72:
case 0x5c73:
case 0x5c74:
case 0x5c75:
case 0x5c76:
case 0x5c77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c80:
case 0x5c81:
case 0x5c82:
case 0x5c83:
case 0x5c84:
case 0x5c85:
case 0x5c86:
case 0x5c87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c88:
case 0x5c89:
case 0x5c8a:
case 0x5c8b:
case 0x5c8c:
case 0x5c8d:
case 0x5c8e:
case 0x5c8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c90:
case 0x5c91:
case 0x5c92:
case 0x5c93:
case 0x5c94:
case 0x5c95:
case 0x5c96:
case 0x5c97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5c98:
case 0x5c99:
case 0x5c9a:
case 0x5c9b:
case 0x5c9c:
case 0x5c9d:
case 0x5c9e:
case 0x5c9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ca0:
case 0x5ca1:
case 0x5ca2:
case 0x5ca3:
case 0x5ca4:
case 0x5ca5:
case 0x5ca6:
case 0x5ca7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ca8:
case 0x5ca9:
case 0x5caa:
case 0x5cab:
case 0x5cac:
case 0x5cad:
case 0x5cae:
case 0x5caf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cb0:
case 0x5cb1:
case 0x5cb2:
case 0x5cb3:
case 0x5cb4:
case 0x5cb5:
case 0x5cb6:
case 0x5cb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cbb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cbc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cc0:
case 0x5cc1:
case 0x5cc2:
case 0x5cc3:
case 0x5cc4:
case 0x5cc5:
case 0x5cc6:
case 0x5cc7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cc8:
case 0x5cc9:
case 0x5cca:
case 0x5ccb:
case 0x5ccc:
case 0x5ccd:
case 0x5cce:
case 0x5ccf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cd0:
case 0x5cd1:
case 0x5cd2:
case 0x5cd3:
case 0x5cd4:
case 0x5cd5:
case 0x5cd6:
case 0x5cd7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cd8:
case 0x5cd9:
case 0x5cda:
case 0x5cdb:
case 0x5cdc:
case 0x5cdd:
case 0x5cde:
case 0x5cdf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ce0:
case 0x5ce1:
case 0x5ce2:
case 0x5ce3:
case 0x5ce4:
case 0x5ce5:
case 0x5ce6:
case 0x5ce7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ce8:
case 0x5ce9:
case 0x5cea:
case 0x5ceb:
case 0x5cec:
case 0x5ced:
case 0x5cee:
case 0x5cef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cf0:
case 0x5cf1:
case 0x5cf2:
case 0x5cf3:
case 0x5cf4:
case 0x5cf5:
case 0x5cf6:
case 0x5cf7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cf8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cf9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cfa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cfb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5cfc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5d00:
case 0x5d01:
case 0x5d02:
case 0x5d03:
case 0x5d04:
case 0x5d05:
case 0x5d06:
case 0x5d07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d08:
case 0x5d09:
case 0x5d0a:
case 0x5d0b:
case 0x5d0c:
case 0x5d0d:
case 0x5d0e:
case 0x5d0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d10:
case 0x5d11:
case 0x5d12:
case 0x5d13:
case 0x5d14:
case 0x5d15:
case 0x5d16:
case 0x5d17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d18:
case 0x5d19:
case 0x5d1a:
case 0x5d1b:
case 0x5d1c:
case 0x5d1d:
case 0x5d1e:
case 0x5d1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d20:
case 0x5d21:
case 0x5d22:
case 0x5d23:
case 0x5d24:
case 0x5d25:
case 0x5d26:
case 0x5d27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d28:
case 0x5d29:
case 0x5d2a:
case 0x5d2b:
case 0x5d2c:
case 0x5d2d:
case 0x5d2e:
case 0x5d2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d30:
case 0x5d31:
case 0x5d32:
case 0x5d33:
case 0x5d34:
case 0x5d35:
case 0x5d36:
case 0x5d37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5d39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5d40:
case 0x5d41:
case 0x5d42:
case 0x5d43:
case 0x5d44:
case 0x5d45:
case 0x5d46:
case 0x5d47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d48:
case 0x5d49:
case 0x5d4a:
case 0x5d4b:
case 0x5d4c:
case 0x5d4d:
case 0x5d4e:
case 0x5d4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d50:
case 0x5d51:
case 0x5d52:
case 0x5d53:
case 0x5d54:
case 0x5d55:
case 0x5d56:
case 0x5d57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d58:
case 0x5d59:
case 0x5d5a:
case 0x5d5b:
case 0x5d5c:
case 0x5d5d:
case 0x5d5e:
case 0x5d5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d60:
case 0x5d61:
case 0x5d62:
case 0x5d63:
case 0x5d64:
case 0x5d65:
case 0x5d66:
case 0x5d67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d68:
case 0x5d69:
case 0x5d6a:
case 0x5d6b:
case 0x5d6c:
case 0x5d6d:
case 0x5d6e:
case 0x5d6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d70:
case 0x5d71:
case 0x5d72:
case 0x5d73:
case 0x5d74:
case 0x5d75:
case 0x5d76:
case 0x5d77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5d79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5d80:
case 0x5d81:
case 0x5d82:
case 0x5d83:
case 0x5d84:
case 0x5d85:
case 0x5d86:
case 0x5d87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d88:
case 0x5d89:
case 0x5d8a:
case 0x5d8b:
case 0x5d8c:
case 0x5d8d:
case 0x5d8e:
case 0x5d8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5d90:
case 0x5d91:
case 0x5d92:
case 0x5d93:
case 0x5d94:
case 0x5d95:
case 0x5d96:
case 0x5d97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5d98:
case 0x5d99:
case 0x5d9a:
case 0x5d9b:
case 0x5d9c:
case 0x5d9d:
case 0x5d9e:
case 0x5d9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5da0:
case 0x5da1:
case 0x5da2:
case 0x5da3:
case 0x5da4:
case 0x5da5:
case 0x5da6:
case 0x5da7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5da8:
case 0x5da9:
case 0x5daa:
case 0x5dab:
case 0x5dac:
case 0x5dad:
case 0x5dae:
case 0x5daf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5db0:
case 0x5db1:
case 0x5db2:
case 0x5db3:
case 0x5db4:
case 0x5db5:
case 0x5db6:
case 0x5db7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5db8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5db9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5dc0:
case 0x5dc1:
case 0x5dc2:
case 0x5dc3:
case 0x5dc4:
case 0x5dc5:
case 0x5dc6:
case 0x5dc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dc8:
case 0x5dc9:
case 0x5dca:
case 0x5dcb:
case 0x5dcc:
case 0x5dcd:
case 0x5dce:
case 0x5dcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dd0:
case 0x5dd1:
case 0x5dd2:
case 0x5dd3:
case 0x5dd4:
case 0x5dd5:
case 0x5dd6:
case 0x5dd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dd8:
case 0x5dd9:
case 0x5dda:
case 0x5ddb:
case 0x5ddc:
case 0x5ddd:
case 0x5dde:
case 0x5ddf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5de0:
case 0x5de1:
case 0x5de2:
case 0x5de3:
case 0x5de4:
case 0x5de5:
case 0x5de6:
case 0x5de7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5de8:
case 0x5de9:
case 0x5dea:
case 0x5deb:
case 0x5dec:
case 0x5ded:
case 0x5dee:
case 0x5def:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5df0:
case 0x5df1:
case 0x5df2:
case 0x5df3:
case 0x5df4:
case 0x5df5:
case 0x5df6:
case 0x5df7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5df8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5df9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dfa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dfb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5dfc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e00:
case 0x5e01:
case 0x5e02:
case 0x5e03:
case 0x5e04:
case 0x5e05:
case 0x5e06:
case 0x5e07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e10:
case 0x5e11:
case 0x5e12:
case 0x5e13:
case 0x5e14:
case 0x5e15:
case 0x5e16:
case 0x5e17:
	{
	 HAM<uint8, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e18:
case 0x5e19:
case 0x5e1a:
case 0x5e1b:
case 0x5e1c:
case 0x5e1d:
case 0x5e1e:
case 0x5e1f:
	{
	 HAM<uint8, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e20:
case 0x5e21:
case 0x5e22:
case 0x5e23:
case 0x5e24:
case 0x5e25:
case 0x5e26:
case 0x5e27:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e28:
case 0x5e29:
case 0x5e2a:
case 0x5e2b:
case 0x5e2c:
case 0x5e2d:
case 0x5e2e:
case 0x5e2f:
	{
	 HAM<uint8, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e30:
case 0x5e31:
case 0x5e32:
case 0x5e33:
case 0x5e34:
case 0x5e35:
case 0x5e36:
case 0x5e37:
	{
	 HAM<uint8, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e38:
	{
	 HAM<uint8, ABS_SHORT> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e39:
	{
	 HAM<uint8, ABS_LONG> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e3a:
	{
	 HAM<uint8, PC_DISP> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e3b:
	{
	 HAM<uint8, PC_INDEX> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e3c:
	{
	 HAM<uint8, IMMEDIATE> src(this); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e40:
case 0x5e41:
case 0x5e42:
case 0x5e43:
case 0x5e44:
case 0x5e45:
case 0x5e46:
case 0x5e47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e48:
case 0x5e49:
case 0x5e4a:
case 0x5e4b:
case 0x5e4c:
case 0x5e4d:
case 0x5e4e:
case 0x5e4f:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e50:
case 0x5e51:
case 0x5e52:
case 0x5e53:
case 0x5e54:
case 0x5e55:
case 0x5e56:
case 0x5e57:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e58:
case 0x5e59:
case 0x5e5a:
case 0x5e5b:
case 0x5e5c:
case 0x5e5d:
case 0x5e5e:
case 0x5e5f:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e60:
case 0x5e61:
case 0x5e62:
case 0x5e63:
case 0x5e64:
case 0x5e65:
case 0x5e66:
case 0x5e67:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e68:
case 0x5e69:
case 0x5e6a:
case 0x5e6b:
case 0x5e6c:
case 0x5e6d:
case 0x5e6e:
case 0x5e6f:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e70:
case 0x5e71:
case 0x5e72:
case 0x5e73:
case 0x5e74:
case 0x5e75:
case 0x5e76:
case 0x5e77:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e78:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e79:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e7a:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e7b:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e7c:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e80:
case 0x5e81:
case 0x5e82:
case 0x5e83:
case 0x5e84:
case 0x5e85:
case 0x5e86:
case 0x5e87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e88:
case 0x5e89:
case 0x5e8a:
case 0x5e8b:
case 0x5e8c:
case 0x5e8d:
case 0x5e8e:
case 0x5e8f:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e90:
case 0x5e91:
case 0x5e92:
case 0x5e93:
case 0x5e94:
case 0x5e95:
case 0x5e96:
case 0x5e97:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5e98:
case 0x5e99:
case 0x5e9a:
case 0x5e9b:
case 0x5e9c:
case 0x5e9d:
case 0x5e9e:
case 0x5e9f:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ea0:
case 0x5ea1:
case 0x5ea2:
case 0x5ea3:
case 0x5ea4:
case 0x5ea5:
case 0x5ea6:
case 0x5ea7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ea8:
case 0x5ea9:
case 0x5eaa:
case 0x5eab:
case 0x5eac:
case 0x5ead:
case 0x5eae:
case 0x5eaf:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5eb0:
case 0x5eb1:
case 0x5eb2:
case 0x5eb3:
case 0x5eb4:
case 0x5eb5:
case 0x5eb6:
case 0x5eb7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5eb8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5eb9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5eba:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ebb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ebc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ec0:
case 0x5ec1:
case 0x5ec2:
case 0x5ec3:
case 0x5ec4:
case 0x5ec5:
case 0x5ec6:
case 0x5ec7:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ec8:
case 0x5ec9:
case 0x5eca:
case 0x5ecb:
case 0x5ecc:
case 0x5ecd:
case 0x5ece:
case 0x5ecf:
	{
	 HAM<uint16, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ed0:
case 0x5ed1:
case 0x5ed2:
case 0x5ed3:
case 0x5ed4:
case 0x5ed5:
case 0x5ed6:
case 0x5ed7:
	{
	 HAM<uint16, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ed8:
case 0x5ed9:
case 0x5eda:
case 0x5edb:
case 0x5edc:
case 0x5edd:
case 0x5ede:
case 0x5edf:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ee0:
case 0x5ee1:
case 0x5ee2:
case 0x5ee3:
case 0x5ee4:
case 0x5ee5:
case 0x5ee6:
case 0x5ee7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ee8:
case 0x5ee9:
case 0x5eea:
case 0x5eeb:
case 0x5eec:
case 0x5eed:
case 0x5eee:
case 0x5eef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ef0:
case 0x5ef1:
case 0x5ef2:
case 0x5ef3:
case 0x5ef4:
case 0x5ef5:
case 0x5ef6:
case 0x5ef7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ef8:
	{
	 HAM<uint16, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ef9:
	{
	 HAM<uint16, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5efa:
	{
	 HAM<uint16, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5efb:
	{
	 HAM<uint16, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5efc:
	{
	 HAM<uint16, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5f00:
case 0x5f01:
case 0x5f02:
case 0x5f03:
case 0x5f04:
case 0x5f05:
case 0x5f06:
case 0x5f07:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint8, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f08:
case 0x5f09:
case 0x5f0a:
case 0x5f0b:
case 0x5f0c:
case 0x5f0d:
case 0x5f0e:
case 0x5f0f:
	{
	 HAM<uint8, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f10:
case 0x5f11:
case 0x5f12:
case 0x5f13:
case 0x5f14:
case 0x5f15:
case 0x5f16:
case 0x5f17:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f18:
case 0x5f19:
case 0x5f1a:
case 0x5f1b:
case 0x5f1c:
case 0x5f1d:
case 0x5f1e:
case 0x5f1f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f20:
case 0x5f21:
case 0x5f22:
case 0x5f23:
case 0x5f24:
case 0x5f25:
case 0x5f26:
case 0x5f27:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f28:
case 0x5f29:
case 0x5f2a:
case 0x5f2b:
case 0x5f2c:
case 0x5f2d:
case 0x5f2e:
case 0x5f2f:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f30:
case 0x5f31:
case 0x5f32:
case 0x5f33:
case 0x5f34:
case 0x5f35:
case 0x5f36:
case 0x5f37:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f38:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5f39:
	{
	 HAM<uint8, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint8, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5f40:
case 0x5f41:
case 0x5f42:
case 0x5f43:
case 0x5f44:
case 0x5f45:
case 0x5f46:
case 0x5f47:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint16, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f48:
case 0x5f49:
case 0x5f4a:
case 0x5f4b:
case 0x5f4c:
case 0x5f4d:
case 0x5f4e:
case 0x5f4f:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f50:
case 0x5f51:
case 0x5f52:
case 0x5f53:
case 0x5f54:
case 0x5f55:
case 0x5f56:
case 0x5f57:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f58:
case 0x5f59:
case 0x5f5a:
case 0x5f5b:
case 0x5f5c:
case 0x5f5d:
case 0x5f5e:
case 0x5f5f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f60:
case 0x5f61:
case 0x5f62:
case 0x5f63:
case 0x5f64:
case 0x5f65:
case 0x5f66:
case 0x5f67:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f68:
case 0x5f69:
case 0x5f6a:
case 0x5f6b:
case 0x5f6c:
case 0x5f6d:
case 0x5f6e:
case 0x5f6f:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f70:
case 0x5f71:
case 0x5f72:
case 0x5f73:
case 0x5f74:
case 0x5f75:
case 0x5f76:
case 0x5f77:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f78:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5f79:
	{
	 HAM<uint16, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint16, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5f80:
case 0x5f81:
case 0x5f82:
case 0x5f83:
case 0x5f84:
case 0x5f85:
case 0x5f86:
case 0x5f87:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, DATA_REG_DIR> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f88:
case 0x5f89:
case 0x5f8a:
case 0x5f8b:
case 0x5f8c:
case 0x5f8d:
case 0x5f8e:
case 0x5f8f:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b11_b9); ADDX(src, dst);
	}
	break;

case 0x5f90:
case 0x5f91:
case 0x5f92:
case 0x5f93:
case 0x5f94:
case 0x5f95:
case 0x5f96:
case 0x5f97:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5f98:
case 0x5f99:
case 0x5f9a:
case 0x5f9b:
case 0x5f9c:
case 0x5f9d:
case 0x5f9e:
case 0x5f9f:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_POST> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5fa0:
case 0x5fa1:
case 0x5fa2:
case 0x5fa3:
case 0x5fa4:
case 0x5fa5:
case 0x5fa6:
case 0x5fa7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_PRE> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5fa8:
case 0x5fa9:
case 0x5faa:
case 0x5fab:
case 0x5fac:
case 0x5fad:
case 0x5fae:
case 0x5faf:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_DISP> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5fb0:
case 0x5fb1:
case 0x5fb2:
case 0x5fb3:
case 0x5fb4:
case 0x5fb5:
case 0x5fb6:
case 0x5fb7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ADDR_REG_INDIR_INDX> dst(this, instr_b2_b0); ADD(src, dst);
	}
	break;

case 0x5fb8:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_SHORT> dst(this); ADD(src, dst);
	}
	break;

case 0x5fb9:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b11_b9); HAM<uint32, ABS_LONG> dst(this); ADD(src, dst);
	}
	break;

case 0x5fc0:
case 0x5fc1:
case 0x5fc2:
case 0x5fc3:
case 0x5fc4:
case 0x5fc5:
case 0x5fc6:
case 0x5fc7:
	{
	 HAM<uint32, DATA_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5fc8:
case 0x5fc9:
case 0x5fca:
case 0x5fcb:
case 0x5fcc:
case 0x5fcd:
case 0x5fce:
case 0x5fcf:
	{
	 HAM<uint32, ADDR_REG_DIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5fd0:
case 0x5fd1:
case 0x5fd2:
case 0x5fd3:
case 0x5fd4:
case 0x5fd5:
case 0x5fd6:
case 0x5fd7:
	{
	 HAM<uint32, ADDR_REG_INDIR> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5fd8:
case 0x5fd9:
case 0x5fda:
case 0x5fdb:
case 0x5fdc:
case 0x5fdd:
case 0x5fde:
case 0x5fdf:
	{
	 HAM<uint32, ADDR_REG_INDIR_POST> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5fe0:
case 0x5fe1:
case 0x5fe2:
case 0x5fe3:
case 0x5fe4:
case 0x5fe5:
case 0x5fe6:
case 0x5fe7:
	{
	 HAM<uint32, ADDR_REG_INDIR_PRE> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5fe8:
case 0x5fe9:
case 0x5fea:
case 0x5feb:
case 0x5fec:
case 0x5fed:
case 0x5fee:
case 0x5fef:
	{
	 HAM<uint32, ADDR_REG_INDIR_DISP> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ff0:
case 0x5ff1:
case 0x5ff2:
case 0x5ff3:
case 0x5ff4:
case 0x5ff5:
case 0x5ff6:
case 0x5ff7:
	{
	 HAM<uint32, ADDR_REG_INDIR_INDX> src(this, instr_b2_b0); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ff8:
	{
	 HAM<uint32, ABS_SHORT> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ff9:
	{
	 HAM<uint32, ABS_LONG> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ffa:
	{
	 HAM<uint32, PC_DISP> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ffb:
	{
	 HAM<uint32, PC_INDEX> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x5ffc:
	{
	 HAM<uint32, IMMEDIATE> src(this); HAM<uint32, ADDR_REG_DIR> dst(this, instr_b11_b9); ADD(src, dst);
	}
	break;

case 0x6000:
case 0x6001:
case 0x6002:
case 0x6003:
case 0x6004:
case 0x6005:
case 0x6006:
case 0x6007:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6008:
case 0x6009:
case 0x600a:
case 0x600b:
case 0x600c:
case 0x600d:
case 0x600e:
case 0x600f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6010:
case 0x6011:
case 0x6012:
case 0x6013:
case 0x6014:
case 0x6015:
case 0x6016:
case 0x6017:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6018:
case 0x6019:
case 0x601a:
case 0x601b:
case 0x601c:
case 0x601d:
case 0x601e:
case 0x601f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6020:
case 0x6021:
case 0x6022:
case 0x6023:
case 0x6024:
case 0x6025:
case 0x6026:
case 0x6027:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6028:
case 0x6029:
case 0x602a:
case 0x602b:
case 0x602c:
case 0x602d:
case 0x602e:
case 0x602f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6030:
case 0x6031:
case 0x6032:
case 0x6033:
case 0x6034:
case 0x6035:
case 0x6036:
case 0x6037:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6038:
case 0x6039:
case 0x603a:
case 0x603b:
case 0x603c:
case 0x603d:
case 0x603e:
case 0x603f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6040:
case 0x6041:
case 0x6042:
case 0x6043:
case 0x6044:
case 0x6045:
case 0x6046:
case 0x6047:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6048:
case 0x6049:
case 0x604a:
case 0x604b:
case 0x604c:
case 0x604d:
case 0x604e:
case 0x604f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6050:
case 0x6051:
case 0x6052:
case 0x6053:
case 0x6054:
case 0x6055:
case 0x6056:
case 0x6057:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6058:
case 0x6059:
case 0x605a:
case 0x605b:
case 0x605c:
case 0x605d:
case 0x605e:
case 0x605f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6060:
case 0x6061:
case 0x6062:
case 0x6063:
case 0x6064:
case 0x6065:
case 0x6066:
case 0x6067:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6068:
case 0x6069:
case 0x606a:
case 0x606b:
case 0x606c:
case 0x606d:
case 0x606e:
case 0x606f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6070:
case 0x6071:
case 0x6072:
case 0x6073:
case 0x6074:
case 0x6075:
case 0x6076:
case 0x6077:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6078:
case 0x6079:
case 0x607a:
case 0x607b:
case 0x607c:
case 0x607d:
case 0x607e:
case 0x607f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6080:
case 0x6081:
case 0x6082:
case 0x6083:
case 0x6084:
case 0x6085:
case 0x6086:
case 0x6087:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6088:
case 0x6089:
case 0x608a:
case 0x608b:
case 0x608c:
case 0x608d:
case 0x608e:
case 0x608f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6090:
case 0x6091:
case 0x6092:
case 0x6093:
case 0x6094:
case 0x6095:
case 0x6096:
case 0x6097:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6098:
case 0x6099:
case 0x609a:
case 0x609b:
case 0x609c:
case 0x609d:
case 0x609e:
case 0x609f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x60a0:
case 0x60a1:
case 0x60a2:
case 0x60a3:
case 0x60a4:
case 0x60a5:
case 0x60a6:
case 0x60a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x60a8:
case 0x60a9:
case 0x60aa:
case 0x60ab:
case 0x60ac:
case 0x60ad:
case 0x60ae:
case 0x60af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x60b0:
case 0x60b1:
case 0x60b2:
case 0x60b3:
case 0x60b4:
case 0x60b5:
case 0x60b6:
case 0x60b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x60b8:
case 0x60b9:
case 0x60ba:
case 0x60bb:
case 0x60bc:
case 0x60bd:
case 0x60be:
case 0x60bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x60d0:
case 0x60d1:
case 0x60d2:
case 0x60d3:
case 0x60d4:
case 0x60d5:
case 0x60d6:
case 0x60d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ASR(targ, 1);
	}
	break;

case 0x60d8:
case 0x60d9:
case 0x60da:
case 0x60db:
case 0x60dc:
case 0x60dd:
case 0x60de:
case 0x60df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ASR(targ, 1);
	}
	break;

case 0x60e0:
case 0x60e1:
case 0x60e2:
case 0x60e3:
case 0x60e4:
case 0x60e5:
case 0x60e6:
case 0x60e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ASR(targ, 1);
	}
	break;

case 0x60e8:
case 0x60e9:
case 0x60ea:
case 0x60eb:
case 0x60ec:
case 0x60ed:
case 0x60ee:
case 0x60ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ASR(targ, 1);
	}
	break;

case 0x60f0:
case 0x60f1:
case 0x60f2:
case 0x60f3:
case 0x60f4:
case 0x60f5:
case 0x60f6:
case 0x60f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ASR(targ, 1);
	}
	break;

case 0x60f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ASR(targ, 1);
	}
	break;

case 0x60f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ASR(targ, 1);
	}
	break;

case 0x6100:
case 0x6101:
case 0x6102:
case 0x6103:
case 0x6104:
case 0x6105:
case 0x6106:
case 0x6107:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6108:
case 0x6109:
case 0x610a:
case 0x610b:
case 0x610c:
case 0x610d:
case 0x610e:
case 0x610f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6110:
case 0x6111:
case 0x6112:
case 0x6113:
case 0x6114:
case 0x6115:
case 0x6116:
case 0x6117:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6118:
case 0x6119:
case 0x611a:
case 0x611b:
case 0x611c:
case 0x611d:
case 0x611e:
case 0x611f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6120:
case 0x6121:
case 0x6122:
case 0x6123:
case 0x6124:
case 0x6125:
case 0x6126:
case 0x6127:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6128:
case 0x6129:
case 0x612a:
case 0x612b:
case 0x612c:
case 0x612d:
case 0x612e:
case 0x612f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6130:
case 0x6131:
case 0x6132:
case 0x6133:
case 0x6134:
case 0x6135:
case 0x6136:
case 0x6137:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6138:
case 0x6139:
case 0x613a:
case 0x613b:
case 0x613c:
case 0x613d:
case 0x613e:
case 0x613f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6140:
case 0x6141:
case 0x6142:
case 0x6143:
case 0x6144:
case 0x6145:
case 0x6146:
case 0x6147:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6148:
case 0x6149:
case 0x614a:
case 0x614b:
case 0x614c:
case 0x614d:
case 0x614e:
case 0x614f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6150:
case 0x6151:
case 0x6152:
case 0x6153:
case 0x6154:
case 0x6155:
case 0x6156:
case 0x6157:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6158:
case 0x6159:
case 0x615a:
case 0x615b:
case 0x615c:
case 0x615d:
case 0x615e:
case 0x615f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6160:
case 0x6161:
case 0x6162:
case 0x6163:
case 0x6164:
case 0x6165:
case 0x6166:
case 0x6167:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6168:
case 0x6169:
case 0x616a:
case 0x616b:
case 0x616c:
case 0x616d:
case 0x616e:
case 0x616f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6170:
case 0x6171:
case 0x6172:
case 0x6173:
case 0x6174:
case 0x6175:
case 0x6176:
case 0x6177:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6178:
case 0x6179:
case 0x617a:
case 0x617b:
case 0x617c:
case 0x617d:
case 0x617e:
case 0x617f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6180:
case 0x6181:
case 0x6182:
case 0x6183:
case 0x6184:
case 0x6185:
case 0x6186:
case 0x6187:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6188:
case 0x6189:
case 0x618a:
case 0x618b:
case 0x618c:
case 0x618d:
case 0x618e:
case 0x618f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6190:
case 0x6191:
case 0x6192:
case 0x6193:
case 0x6194:
case 0x6195:
case 0x6196:
case 0x6197:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6198:
case 0x6199:
case 0x619a:
case 0x619b:
case 0x619c:
case 0x619d:
case 0x619e:
case 0x619f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x61a0:
case 0x61a1:
case 0x61a2:
case 0x61a3:
case 0x61a4:
case 0x61a5:
case 0x61a6:
case 0x61a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x61a8:
case 0x61a9:
case 0x61aa:
case 0x61ab:
case 0x61ac:
case 0x61ad:
case 0x61ae:
case 0x61af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x61b0:
case 0x61b1:
case 0x61b2:
case 0x61b3:
case 0x61b4:
case 0x61b5:
case 0x61b6:
case 0x61b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x61b8:
case 0x61b9:
case 0x61ba:
case 0x61bb:
case 0x61bc:
case 0x61bd:
case 0x61be:
case 0x61bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x61d0:
case 0x61d1:
case 0x61d2:
case 0x61d3:
case 0x61d4:
case 0x61d5:
case 0x61d6:
case 0x61d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ASL(targ, 1);
	}
	break;

case 0x61d8:
case 0x61d9:
case 0x61da:
case 0x61db:
case 0x61dc:
case 0x61dd:
case 0x61de:
case 0x61df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ASL(targ, 1);
	}
	break;

case 0x61e0:
case 0x61e1:
case 0x61e2:
case 0x61e3:
case 0x61e4:
case 0x61e5:
case 0x61e6:
case 0x61e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ASL(targ, 1);
	}
	break;

case 0x61e8:
case 0x61e9:
case 0x61ea:
case 0x61eb:
case 0x61ec:
case 0x61ed:
case 0x61ee:
case 0x61ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ASL(targ, 1);
	}
	break;

case 0x61f0:
case 0x61f1:
case 0x61f2:
case 0x61f3:
case 0x61f4:
case 0x61f5:
case 0x61f6:
case 0x61f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ASL(targ, 1);
	}
	break;

case 0x61f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ASL(targ, 1);
	}
	break;

case 0x61f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ASL(targ, 1);
	}
	break;

case 0x6200:
case 0x6201:
case 0x6202:
case 0x6203:
case 0x6204:
case 0x6205:
case 0x6206:
case 0x6207:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6208:
case 0x6209:
case 0x620a:
case 0x620b:
case 0x620c:
case 0x620d:
case 0x620e:
case 0x620f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6210:
case 0x6211:
case 0x6212:
case 0x6213:
case 0x6214:
case 0x6215:
case 0x6216:
case 0x6217:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6218:
case 0x6219:
case 0x621a:
case 0x621b:
case 0x621c:
case 0x621d:
case 0x621e:
case 0x621f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6220:
case 0x6221:
case 0x6222:
case 0x6223:
case 0x6224:
case 0x6225:
case 0x6226:
case 0x6227:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6228:
case 0x6229:
case 0x622a:
case 0x622b:
case 0x622c:
case 0x622d:
case 0x622e:
case 0x622f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6230:
case 0x6231:
case 0x6232:
case 0x6233:
case 0x6234:
case 0x6235:
case 0x6236:
case 0x6237:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6238:
case 0x6239:
case 0x623a:
case 0x623b:
case 0x623c:
case 0x623d:
case 0x623e:
case 0x623f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6240:
case 0x6241:
case 0x6242:
case 0x6243:
case 0x6244:
case 0x6245:
case 0x6246:
case 0x6247:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6248:
case 0x6249:
case 0x624a:
case 0x624b:
case 0x624c:
case 0x624d:
case 0x624e:
case 0x624f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6250:
case 0x6251:
case 0x6252:
case 0x6253:
case 0x6254:
case 0x6255:
case 0x6256:
case 0x6257:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6258:
case 0x6259:
case 0x625a:
case 0x625b:
case 0x625c:
case 0x625d:
case 0x625e:
case 0x625f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6260:
case 0x6261:
case 0x6262:
case 0x6263:
case 0x6264:
case 0x6265:
case 0x6266:
case 0x6267:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6268:
case 0x6269:
case 0x626a:
case 0x626b:
case 0x626c:
case 0x626d:
case 0x626e:
case 0x626f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6270:
case 0x6271:
case 0x6272:
case 0x6273:
case 0x6274:
case 0x6275:
case 0x6276:
case 0x6277:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6278:
case 0x6279:
case 0x627a:
case 0x627b:
case 0x627c:
case 0x627d:
case 0x627e:
case 0x627f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6280:
case 0x6281:
case 0x6282:
case 0x6283:
case 0x6284:
case 0x6285:
case 0x6286:
case 0x6287:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6288:
case 0x6289:
case 0x628a:
case 0x628b:
case 0x628c:
case 0x628d:
case 0x628e:
case 0x628f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6290:
case 0x6291:
case 0x6292:
case 0x6293:
case 0x6294:
case 0x6295:
case 0x6296:
case 0x6297:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6298:
case 0x6299:
case 0x629a:
case 0x629b:
case 0x629c:
case 0x629d:
case 0x629e:
case 0x629f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x62a0:
case 0x62a1:
case 0x62a2:
case 0x62a3:
case 0x62a4:
case 0x62a5:
case 0x62a6:
case 0x62a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x62a8:
case 0x62a9:
case 0x62aa:
case 0x62ab:
case 0x62ac:
case 0x62ad:
case 0x62ae:
case 0x62af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x62b0:
case 0x62b1:
case 0x62b2:
case 0x62b3:
case 0x62b4:
case 0x62b5:
case 0x62b6:
case 0x62b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x62b8:
case 0x62b9:
case 0x62ba:
case 0x62bb:
case 0x62bc:
case 0x62bd:
case 0x62be:
case 0x62bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x62d0:
case 0x62d1:
case 0x62d2:
case 0x62d3:
case 0x62d4:
case 0x62d5:
case 0x62d6:
case 0x62d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); LSR(targ, 1);
	}
	break;

case 0x62d8:
case 0x62d9:
case 0x62da:
case 0x62db:
case 0x62dc:
case 0x62dd:
case 0x62de:
case 0x62df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); LSR(targ, 1);
	}
	break;

case 0x62e0:
case 0x62e1:
case 0x62e2:
case 0x62e3:
case 0x62e4:
case 0x62e5:
case 0x62e6:
case 0x62e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); LSR(targ, 1);
	}
	break;

case 0x62e8:
case 0x62e9:
case 0x62ea:
case 0x62eb:
case 0x62ec:
case 0x62ed:
case 0x62ee:
case 0x62ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); LSR(targ, 1);
	}
	break;

case 0x62f0:
case 0x62f1:
case 0x62f2:
case 0x62f3:
case 0x62f4:
case 0x62f5:
case 0x62f6:
case 0x62f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); LSR(targ, 1);
	}
	break;

case 0x62f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); LSR(targ, 1);
	}
	break;

case 0x62f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); LSR(targ, 1);
	}
	break;

case 0x6300:
case 0x6301:
case 0x6302:
case 0x6303:
case 0x6304:
case 0x6305:
case 0x6306:
case 0x6307:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6308:
case 0x6309:
case 0x630a:
case 0x630b:
case 0x630c:
case 0x630d:
case 0x630e:
case 0x630f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6310:
case 0x6311:
case 0x6312:
case 0x6313:
case 0x6314:
case 0x6315:
case 0x6316:
case 0x6317:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6318:
case 0x6319:
case 0x631a:
case 0x631b:
case 0x631c:
case 0x631d:
case 0x631e:
case 0x631f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6320:
case 0x6321:
case 0x6322:
case 0x6323:
case 0x6324:
case 0x6325:
case 0x6326:
case 0x6327:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6328:
case 0x6329:
case 0x632a:
case 0x632b:
case 0x632c:
case 0x632d:
case 0x632e:
case 0x632f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6330:
case 0x6331:
case 0x6332:
case 0x6333:
case 0x6334:
case 0x6335:
case 0x6336:
case 0x6337:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6338:
case 0x6339:
case 0x633a:
case 0x633b:
case 0x633c:
case 0x633d:
case 0x633e:
case 0x633f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6340:
case 0x6341:
case 0x6342:
case 0x6343:
case 0x6344:
case 0x6345:
case 0x6346:
case 0x6347:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6348:
case 0x6349:
case 0x634a:
case 0x634b:
case 0x634c:
case 0x634d:
case 0x634e:
case 0x634f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6350:
case 0x6351:
case 0x6352:
case 0x6353:
case 0x6354:
case 0x6355:
case 0x6356:
case 0x6357:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6358:
case 0x6359:
case 0x635a:
case 0x635b:
case 0x635c:
case 0x635d:
case 0x635e:
case 0x635f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6360:
case 0x6361:
case 0x6362:
case 0x6363:
case 0x6364:
case 0x6365:
case 0x6366:
case 0x6367:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6368:
case 0x6369:
case 0x636a:
case 0x636b:
case 0x636c:
case 0x636d:
case 0x636e:
case 0x636f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6370:
case 0x6371:
case 0x6372:
case 0x6373:
case 0x6374:
case 0x6375:
case 0x6376:
case 0x6377:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6378:
case 0x6379:
case 0x637a:
case 0x637b:
case 0x637c:
case 0x637d:
case 0x637e:
case 0x637f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6380:
case 0x6381:
case 0x6382:
case 0x6383:
case 0x6384:
case 0x6385:
case 0x6386:
case 0x6387:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6388:
case 0x6389:
case 0x638a:
case 0x638b:
case 0x638c:
case 0x638d:
case 0x638e:
case 0x638f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6390:
case 0x6391:
case 0x6392:
case 0x6393:
case 0x6394:
case 0x6395:
case 0x6396:
case 0x6397:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6398:
case 0x6399:
case 0x639a:
case 0x639b:
case 0x639c:
case 0x639d:
case 0x639e:
case 0x639f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x63a0:
case 0x63a1:
case 0x63a2:
case 0x63a3:
case 0x63a4:
case 0x63a5:
case 0x63a6:
case 0x63a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x63a8:
case 0x63a9:
case 0x63aa:
case 0x63ab:
case 0x63ac:
case 0x63ad:
case 0x63ae:
case 0x63af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x63b0:
case 0x63b1:
case 0x63b2:
case 0x63b3:
case 0x63b4:
case 0x63b5:
case 0x63b6:
case 0x63b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x63b8:
case 0x63b9:
case 0x63ba:
case 0x63bb:
case 0x63bc:
case 0x63bd:
case 0x63be:
case 0x63bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x63d0:
case 0x63d1:
case 0x63d2:
case 0x63d3:
case 0x63d4:
case 0x63d5:
case 0x63d6:
case 0x63d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); LSL(targ, 1);
	}
	break;

case 0x63d8:
case 0x63d9:
case 0x63da:
case 0x63db:
case 0x63dc:
case 0x63dd:
case 0x63de:
case 0x63df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); LSL(targ, 1);
	}
	break;

case 0x63e0:
case 0x63e1:
case 0x63e2:
case 0x63e3:
case 0x63e4:
case 0x63e5:
case 0x63e6:
case 0x63e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); LSL(targ, 1);
	}
	break;

case 0x63e8:
case 0x63e9:
case 0x63ea:
case 0x63eb:
case 0x63ec:
case 0x63ed:
case 0x63ee:
case 0x63ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); LSL(targ, 1);
	}
	break;

case 0x63f0:
case 0x63f1:
case 0x63f2:
case 0x63f3:
case 0x63f4:
case 0x63f5:
case 0x63f6:
case 0x63f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); LSL(targ, 1);
	}
	break;

case 0x63f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); LSL(targ, 1);
	}
	break;

case 0x63f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); LSL(targ, 1);
	}
	break;

case 0x6400:
case 0x6401:
case 0x6402:
case 0x6403:
case 0x6404:
case 0x6405:
case 0x6406:
case 0x6407:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6408:
case 0x6409:
case 0x640a:
case 0x640b:
case 0x640c:
case 0x640d:
case 0x640e:
case 0x640f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6410:
case 0x6411:
case 0x6412:
case 0x6413:
case 0x6414:
case 0x6415:
case 0x6416:
case 0x6417:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6418:
case 0x6419:
case 0x641a:
case 0x641b:
case 0x641c:
case 0x641d:
case 0x641e:
case 0x641f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6420:
case 0x6421:
case 0x6422:
case 0x6423:
case 0x6424:
case 0x6425:
case 0x6426:
case 0x6427:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6428:
case 0x6429:
case 0x642a:
case 0x642b:
case 0x642c:
case 0x642d:
case 0x642e:
case 0x642f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6430:
case 0x6431:
case 0x6432:
case 0x6433:
case 0x6434:
case 0x6435:
case 0x6436:
case 0x6437:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6438:
case 0x6439:
case 0x643a:
case 0x643b:
case 0x643c:
case 0x643d:
case 0x643e:
case 0x643f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6440:
case 0x6441:
case 0x6442:
case 0x6443:
case 0x6444:
case 0x6445:
case 0x6446:
case 0x6447:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6448:
case 0x6449:
case 0x644a:
case 0x644b:
case 0x644c:
case 0x644d:
case 0x644e:
case 0x644f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6450:
case 0x6451:
case 0x6452:
case 0x6453:
case 0x6454:
case 0x6455:
case 0x6456:
case 0x6457:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6458:
case 0x6459:
case 0x645a:
case 0x645b:
case 0x645c:
case 0x645d:
case 0x645e:
case 0x645f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6460:
case 0x6461:
case 0x6462:
case 0x6463:
case 0x6464:
case 0x6465:
case 0x6466:
case 0x6467:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6468:
case 0x6469:
case 0x646a:
case 0x646b:
case 0x646c:
case 0x646d:
case 0x646e:
case 0x646f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6470:
case 0x6471:
case 0x6472:
case 0x6473:
case 0x6474:
case 0x6475:
case 0x6476:
case 0x6477:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6478:
case 0x6479:
case 0x647a:
case 0x647b:
case 0x647c:
case 0x647d:
case 0x647e:
case 0x647f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6480:
case 0x6481:
case 0x6482:
case 0x6483:
case 0x6484:
case 0x6485:
case 0x6486:
case 0x6487:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6488:
case 0x6489:
case 0x648a:
case 0x648b:
case 0x648c:
case 0x648d:
case 0x648e:
case 0x648f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6490:
case 0x6491:
case 0x6492:
case 0x6493:
case 0x6494:
case 0x6495:
case 0x6496:
case 0x6497:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6498:
case 0x6499:
case 0x649a:
case 0x649b:
case 0x649c:
case 0x649d:
case 0x649e:
case 0x649f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x64a0:
case 0x64a1:
case 0x64a2:
case 0x64a3:
case 0x64a4:
case 0x64a5:
case 0x64a6:
case 0x64a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x64a8:
case 0x64a9:
case 0x64aa:
case 0x64ab:
case 0x64ac:
case 0x64ad:
case 0x64ae:
case 0x64af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x64b0:
case 0x64b1:
case 0x64b2:
case 0x64b3:
case 0x64b4:
case 0x64b5:
case 0x64b6:
case 0x64b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x64b8:
case 0x64b9:
case 0x64ba:
case 0x64bb:
case 0x64bc:
case 0x64bd:
case 0x64be:
case 0x64bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x64d0:
case 0x64d1:
case 0x64d2:
case 0x64d3:
case 0x64d4:
case 0x64d5:
case 0x64d6:
case 0x64d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ROXR(targ, 1);
	}
	break;

case 0x64d8:
case 0x64d9:
case 0x64da:
case 0x64db:
case 0x64dc:
case 0x64dd:
case 0x64de:
case 0x64df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ROXR(targ, 1);
	}
	break;

case 0x64e0:
case 0x64e1:
case 0x64e2:
case 0x64e3:
case 0x64e4:
case 0x64e5:
case 0x64e6:
case 0x64e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ROXR(targ, 1);
	}
	break;

case 0x64e8:
case 0x64e9:
case 0x64ea:
case 0x64eb:
case 0x64ec:
case 0x64ed:
case 0x64ee:
case 0x64ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ROXR(targ, 1);
	}
	break;

case 0x64f0:
case 0x64f1:
case 0x64f2:
case 0x64f3:
case 0x64f4:
case 0x64f5:
case 0x64f6:
case 0x64f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ROXR(targ, 1);
	}
	break;

case 0x64f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ROXR(targ, 1);
	}
	break;

case 0x64f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ROXR(targ, 1);
	}
	break;

case 0x6500:
case 0x6501:
case 0x6502:
case 0x6503:
case 0x6504:
case 0x6505:
case 0x6506:
case 0x6507:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6508:
case 0x6509:
case 0x650a:
case 0x650b:
case 0x650c:
case 0x650d:
case 0x650e:
case 0x650f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6510:
case 0x6511:
case 0x6512:
case 0x6513:
case 0x6514:
case 0x6515:
case 0x6516:
case 0x6517:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6518:
case 0x6519:
case 0x651a:
case 0x651b:
case 0x651c:
case 0x651d:
case 0x651e:
case 0x651f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6520:
case 0x6521:
case 0x6522:
case 0x6523:
case 0x6524:
case 0x6525:
case 0x6526:
case 0x6527:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6528:
case 0x6529:
case 0x652a:
case 0x652b:
case 0x652c:
case 0x652d:
case 0x652e:
case 0x652f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6530:
case 0x6531:
case 0x6532:
case 0x6533:
case 0x6534:
case 0x6535:
case 0x6536:
case 0x6537:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6538:
case 0x6539:
case 0x653a:
case 0x653b:
case 0x653c:
case 0x653d:
case 0x653e:
case 0x653f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6540:
case 0x6541:
case 0x6542:
case 0x6543:
case 0x6544:
case 0x6545:
case 0x6546:
case 0x6547:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6548:
case 0x6549:
case 0x654a:
case 0x654b:
case 0x654c:
case 0x654d:
case 0x654e:
case 0x654f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6550:
case 0x6551:
case 0x6552:
case 0x6553:
case 0x6554:
case 0x6555:
case 0x6556:
case 0x6557:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6558:
case 0x6559:
case 0x655a:
case 0x655b:
case 0x655c:
case 0x655d:
case 0x655e:
case 0x655f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6560:
case 0x6561:
case 0x6562:
case 0x6563:
case 0x6564:
case 0x6565:
case 0x6566:
case 0x6567:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6568:
case 0x6569:
case 0x656a:
case 0x656b:
case 0x656c:
case 0x656d:
case 0x656e:
case 0x656f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6570:
case 0x6571:
case 0x6572:
case 0x6573:
case 0x6574:
case 0x6575:
case 0x6576:
case 0x6577:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6578:
case 0x6579:
case 0x657a:
case 0x657b:
case 0x657c:
case 0x657d:
case 0x657e:
case 0x657f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6580:
case 0x6581:
case 0x6582:
case 0x6583:
case 0x6584:
case 0x6585:
case 0x6586:
case 0x6587:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6588:
case 0x6589:
case 0x658a:
case 0x658b:
case 0x658c:
case 0x658d:
case 0x658e:
case 0x658f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6590:
case 0x6591:
case 0x6592:
case 0x6593:
case 0x6594:
case 0x6595:
case 0x6596:
case 0x6597:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6598:
case 0x6599:
case 0x659a:
case 0x659b:
case 0x659c:
case 0x659d:
case 0x659e:
case 0x659f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x65a0:
case 0x65a1:
case 0x65a2:
case 0x65a3:
case 0x65a4:
case 0x65a5:
case 0x65a6:
case 0x65a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x65a8:
case 0x65a9:
case 0x65aa:
case 0x65ab:
case 0x65ac:
case 0x65ad:
case 0x65ae:
case 0x65af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x65b0:
case 0x65b1:
case 0x65b2:
case 0x65b3:
case 0x65b4:
case 0x65b5:
case 0x65b6:
case 0x65b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x65b8:
case 0x65b9:
case 0x65ba:
case 0x65bb:
case 0x65bc:
case 0x65bd:
case 0x65be:
case 0x65bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x65d0:
case 0x65d1:
case 0x65d2:
case 0x65d3:
case 0x65d4:
case 0x65d5:
case 0x65d6:
case 0x65d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ROXL(targ, 1);
	}
	break;

case 0x65d8:
case 0x65d9:
case 0x65da:
case 0x65db:
case 0x65dc:
case 0x65dd:
case 0x65de:
case 0x65df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ROXL(targ, 1);
	}
	break;

case 0x65e0:
case 0x65e1:
case 0x65e2:
case 0x65e3:
case 0x65e4:
case 0x65e5:
case 0x65e6:
case 0x65e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ROXL(targ, 1);
	}
	break;

case 0x65e8:
case 0x65e9:
case 0x65ea:
case 0x65eb:
case 0x65ec:
case 0x65ed:
case 0x65ee:
case 0x65ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ROXL(targ, 1);
	}
	break;

case 0x65f0:
case 0x65f1:
case 0x65f2:
case 0x65f3:
case 0x65f4:
case 0x65f5:
case 0x65f6:
case 0x65f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ROXL(targ, 1);
	}
	break;

case 0x65f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ROXL(targ, 1);
	}
	break;

case 0x65f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ROXL(targ, 1);
	}
	break;

case 0x6600:
case 0x6601:
case 0x6602:
case 0x6603:
case 0x6604:
case 0x6605:
case 0x6606:
case 0x6607:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6608:
case 0x6609:
case 0x660a:
case 0x660b:
case 0x660c:
case 0x660d:
case 0x660e:
case 0x660f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6610:
case 0x6611:
case 0x6612:
case 0x6613:
case 0x6614:
case 0x6615:
case 0x6616:
case 0x6617:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6618:
case 0x6619:
case 0x661a:
case 0x661b:
case 0x661c:
case 0x661d:
case 0x661e:
case 0x661f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6620:
case 0x6621:
case 0x6622:
case 0x6623:
case 0x6624:
case 0x6625:
case 0x6626:
case 0x6627:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6628:
case 0x6629:
case 0x662a:
case 0x662b:
case 0x662c:
case 0x662d:
case 0x662e:
case 0x662f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6630:
case 0x6631:
case 0x6632:
case 0x6633:
case 0x6634:
case 0x6635:
case 0x6636:
case 0x6637:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6638:
case 0x6639:
case 0x663a:
case 0x663b:
case 0x663c:
case 0x663d:
case 0x663e:
case 0x663f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6640:
case 0x6641:
case 0x6642:
case 0x6643:
case 0x6644:
case 0x6645:
case 0x6646:
case 0x6647:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6648:
case 0x6649:
case 0x664a:
case 0x664b:
case 0x664c:
case 0x664d:
case 0x664e:
case 0x664f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6650:
case 0x6651:
case 0x6652:
case 0x6653:
case 0x6654:
case 0x6655:
case 0x6656:
case 0x6657:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6658:
case 0x6659:
case 0x665a:
case 0x665b:
case 0x665c:
case 0x665d:
case 0x665e:
case 0x665f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6660:
case 0x6661:
case 0x6662:
case 0x6663:
case 0x6664:
case 0x6665:
case 0x6666:
case 0x6667:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6668:
case 0x6669:
case 0x666a:
case 0x666b:
case 0x666c:
case 0x666d:
case 0x666e:
case 0x666f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6670:
case 0x6671:
case 0x6672:
case 0x6673:
case 0x6674:
case 0x6675:
case 0x6676:
case 0x6677:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6678:
case 0x6679:
case 0x667a:
case 0x667b:
case 0x667c:
case 0x667d:
case 0x667e:
case 0x667f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6680:
case 0x6681:
case 0x6682:
case 0x6683:
case 0x6684:
case 0x6685:
case 0x6686:
case 0x6687:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6688:
case 0x6689:
case 0x668a:
case 0x668b:
case 0x668c:
case 0x668d:
case 0x668e:
case 0x668f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6690:
case 0x6691:
case 0x6692:
case 0x6693:
case 0x6694:
case 0x6695:
case 0x6696:
case 0x6697:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6698:
case 0x6699:
case 0x669a:
case 0x669b:
case 0x669c:
case 0x669d:
case 0x669e:
case 0x669f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x66a0:
case 0x66a1:
case 0x66a2:
case 0x66a3:
case 0x66a4:
case 0x66a5:
case 0x66a6:
case 0x66a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x66a8:
case 0x66a9:
case 0x66aa:
case 0x66ab:
case 0x66ac:
case 0x66ad:
case 0x66ae:
case 0x66af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x66b0:
case 0x66b1:
case 0x66b2:
case 0x66b3:
case 0x66b4:
case 0x66b5:
case 0x66b6:
case 0x66b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x66b8:
case 0x66b9:
case 0x66ba:
case 0x66bb:
case 0x66bc:
case 0x66bd:
case 0x66be:
case 0x66bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x66d0:
case 0x66d1:
case 0x66d2:
case 0x66d3:
case 0x66d4:
case 0x66d5:
case 0x66d6:
case 0x66d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ROR(targ, 1);
	}
	break;

case 0x66d8:
case 0x66d9:
case 0x66da:
case 0x66db:
case 0x66dc:
case 0x66dd:
case 0x66de:
case 0x66df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ROR(targ, 1);
	}
	break;

case 0x66e0:
case 0x66e1:
case 0x66e2:
case 0x66e3:
case 0x66e4:
case 0x66e5:
case 0x66e6:
case 0x66e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ROR(targ, 1);
	}
	break;

case 0x66e8:
case 0x66e9:
case 0x66ea:
case 0x66eb:
case 0x66ec:
case 0x66ed:
case 0x66ee:
case 0x66ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ROR(targ, 1);
	}
	break;

case 0x66f0:
case 0x66f1:
case 0x66f2:
case 0x66f3:
case 0x66f4:
case 0x66f5:
case 0x66f6:
case 0x66f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ROR(targ, 1);
	}
	break;

case 0x66f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ROR(targ, 1);
	}
	break;

case 0x66f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ROR(targ, 1);
	}
	break;

case 0x6700:
case 0x6701:
case 0x6702:
case 0x6703:
case 0x6704:
case 0x6705:
case 0x6706:
case 0x6707:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6708:
case 0x6709:
case 0x670a:
case 0x670b:
case 0x670c:
case 0x670d:
case 0x670e:
case 0x670f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6710:
case 0x6711:
case 0x6712:
case 0x6713:
case 0x6714:
case 0x6715:
case 0x6716:
case 0x6717:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6718:
case 0x6719:
case 0x671a:
case 0x671b:
case 0x671c:
case 0x671d:
case 0x671e:
case 0x671f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6720:
case 0x6721:
case 0x6722:
case 0x6723:
case 0x6724:
case 0x6725:
case 0x6726:
case 0x6727:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6728:
case 0x6729:
case 0x672a:
case 0x672b:
case 0x672c:
case 0x672d:
case 0x672e:
case 0x672f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6730:
case 0x6731:
case 0x6732:
case 0x6733:
case 0x6734:
case 0x6735:
case 0x6736:
case 0x6737:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6738:
case 0x6739:
case 0x673a:
case 0x673b:
case 0x673c:
case 0x673d:
case 0x673e:
case 0x673f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6740:
case 0x6741:
case 0x6742:
case 0x6743:
case 0x6744:
case 0x6745:
case 0x6746:
case 0x6747:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6748:
case 0x6749:
case 0x674a:
case 0x674b:
case 0x674c:
case 0x674d:
case 0x674e:
case 0x674f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6750:
case 0x6751:
case 0x6752:
case 0x6753:
case 0x6754:
case 0x6755:
case 0x6756:
case 0x6757:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6758:
case 0x6759:
case 0x675a:
case 0x675b:
case 0x675c:
case 0x675d:
case 0x675e:
case 0x675f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6760:
case 0x6761:
case 0x6762:
case 0x6763:
case 0x6764:
case 0x6765:
case 0x6766:
case 0x6767:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6768:
case 0x6769:
case 0x676a:
case 0x676b:
case 0x676c:
case 0x676d:
case 0x676e:
case 0x676f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6770:
case 0x6771:
case 0x6772:
case 0x6773:
case 0x6774:
case 0x6775:
case 0x6776:
case 0x6777:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6778:
case 0x6779:
case 0x677a:
case 0x677b:
case 0x677c:
case 0x677d:
case 0x677e:
case 0x677f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6780:
case 0x6781:
case 0x6782:
case 0x6783:
case 0x6784:
case 0x6785:
case 0x6786:
case 0x6787:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6788:
case 0x6789:
case 0x678a:
case 0x678b:
case 0x678c:
case 0x678d:
case 0x678e:
case 0x678f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6790:
case 0x6791:
case 0x6792:
case 0x6793:
case 0x6794:
case 0x6795:
case 0x6796:
case 0x6797:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6798:
case 0x6799:
case 0x679a:
case 0x679b:
case 0x679c:
case 0x679d:
case 0x679e:
case 0x679f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x67a0:
case 0x67a1:
case 0x67a2:
case 0x67a3:
case 0x67a4:
case 0x67a5:
case 0x67a6:
case 0x67a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x67a8:
case 0x67a9:
case 0x67aa:
case 0x67ab:
case 0x67ac:
case 0x67ad:
case 0x67ae:
case 0x67af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x67b0:
case 0x67b1:
case 0x67b2:
case 0x67b3:
case 0x67b4:
case 0x67b5:
case 0x67b6:
case 0x67b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x67b8:
case 0x67b9:
case 0x67ba:
case 0x67bb:
case 0x67bc:
case 0x67bd:
case 0x67be:
case 0x67bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x67d0:
case 0x67d1:
case 0x67d2:
case 0x67d3:
case 0x67d4:
case 0x67d5:
case 0x67d6:
case 0x67d7:
	{
	 HAM<uint16, ADDR_REG_INDIR> targ(this, instr_b2_b0); ROL(targ, 1);
	}
	break;

case 0x67d8:
case 0x67d9:
case 0x67da:
case 0x67db:
case 0x67dc:
case 0x67dd:
case 0x67de:
case 0x67df:
	{
	 HAM<uint16, ADDR_REG_INDIR_POST> targ(this, instr_b2_b0); ROL(targ, 1);
	}
	break;

case 0x67e0:
case 0x67e1:
case 0x67e2:
case 0x67e3:
case 0x67e4:
case 0x67e5:
case 0x67e6:
case 0x67e7:
	{
	 HAM<uint16, ADDR_REG_INDIR_PRE> targ(this, instr_b2_b0); ROL(targ, 1);
	}
	break;

case 0x67e8:
case 0x67e9:
case 0x67ea:
case 0x67eb:
case 0x67ec:
case 0x67ed:
case 0x67ee:
case 0x67ef:
	{
	 HAM<uint16, ADDR_REG_INDIR_DISP> targ(this, instr_b2_b0); ROL(targ, 1);
	}
	break;

case 0x67f0:
case 0x67f1:
case 0x67f2:
case 0x67f3:
case 0x67f4:
case 0x67f5:
case 0x67f6:
case 0x67f7:
	{
	 HAM<uint16, ADDR_REG_INDIR_INDX> targ(this, instr_b2_b0); ROL(targ, 1);
	}
	break;

case 0x67f8:
	{
	 HAM<uint16, ABS_SHORT> targ(this); ROL(targ, 1);
	}
	break;

case 0x67f9:
	{
	 HAM<uint16, ABS_LONG> targ(this); ROL(targ, 1);
	}
	break;

case 0x6800:
case 0x6801:
case 0x6802:
case 0x6803:
case 0x6804:
case 0x6805:
case 0x6806:
case 0x6807:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6808:
case 0x6809:
case 0x680a:
case 0x680b:
case 0x680c:
case 0x680d:
case 0x680e:
case 0x680f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6810:
case 0x6811:
case 0x6812:
case 0x6813:
case 0x6814:
case 0x6815:
case 0x6816:
case 0x6817:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6818:
case 0x6819:
case 0x681a:
case 0x681b:
case 0x681c:
case 0x681d:
case 0x681e:
case 0x681f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6820:
case 0x6821:
case 0x6822:
case 0x6823:
case 0x6824:
case 0x6825:
case 0x6826:
case 0x6827:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6828:
case 0x6829:
case 0x682a:
case 0x682b:
case 0x682c:
case 0x682d:
case 0x682e:
case 0x682f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6830:
case 0x6831:
case 0x6832:
case 0x6833:
case 0x6834:
case 0x6835:
case 0x6836:
case 0x6837:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6838:
case 0x6839:
case 0x683a:
case 0x683b:
case 0x683c:
case 0x683d:
case 0x683e:
case 0x683f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6840:
case 0x6841:
case 0x6842:
case 0x6843:
case 0x6844:
case 0x6845:
case 0x6846:
case 0x6847:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6848:
case 0x6849:
case 0x684a:
case 0x684b:
case 0x684c:
case 0x684d:
case 0x684e:
case 0x684f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6850:
case 0x6851:
case 0x6852:
case 0x6853:
case 0x6854:
case 0x6855:
case 0x6856:
case 0x6857:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6858:
case 0x6859:
case 0x685a:
case 0x685b:
case 0x685c:
case 0x685d:
case 0x685e:
case 0x685f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6860:
case 0x6861:
case 0x6862:
case 0x6863:
case 0x6864:
case 0x6865:
case 0x6866:
case 0x6867:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6868:
case 0x6869:
case 0x686a:
case 0x686b:
case 0x686c:
case 0x686d:
case 0x686e:
case 0x686f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6870:
case 0x6871:
case 0x6872:
case 0x6873:
case 0x6874:
case 0x6875:
case 0x6876:
case 0x6877:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6878:
case 0x6879:
case 0x687a:
case 0x687b:
case 0x687c:
case 0x687d:
case 0x687e:
case 0x687f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6880:
case 0x6881:
case 0x6882:
case 0x6883:
case 0x6884:
case 0x6885:
case 0x6886:
case 0x6887:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6888:
case 0x6889:
case 0x688a:
case 0x688b:
case 0x688c:
case 0x688d:
case 0x688e:
case 0x688f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6890:
case 0x6891:
case 0x6892:
case 0x6893:
case 0x6894:
case 0x6895:
case 0x6896:
case 0x6897:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6898:
case 0x6899:
case 0x689a:
case 0x689b:
case 0x689c:
case 0x689d:
case 0x689e:
case 0x689f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x68a0:
case 0x68a1:
case 0x68a2:
case 0x68a3:
case 0x68a4:
case 0x68a5:
case 0x68a6:
case 0x68a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x68a8:
case 0x68a9:
case 0x68aa:
case 0x68ab:
case 0x68ac:
case 0x68ad:
case 0x68ae:
case 0x68af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x68b0:
case 0x68b1:
case 0x68b2:
case 0x68b3:
case 0x68b4:
case 0x68b5:
case 0x68b6:
case 0x68b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x68b8:
case 0x68b9:
case 0x68ba:
case 0x68bb:
case 0x68bc:
case 0x68bd:
case 0x68be:
case 0x68bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6900:
case 0x6901:
case 0x6902:
case 0x6903:
case 0x6904:
case 0x6905:
case 0x6906:
case 0x6907:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6908:
case 0x6909:
case 0x690a:
case 0x690b:
case 0x690c:
case 0x690d:
case 0x690e:
case 0x690f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6910:
case 0x6911:
case 0x6912:
case 0x6913:
case 0x6914:
case 0x6915:
case 0x6916:
case 0x6917:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6918:
case 0x6919:
case 0x691a:
case 0x691b:
case 0x691c:
case 0x691d:
case 0x691e:
case 0x691f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6920:
case 0x6921:
case 0x6922:
case 0x6923:
case 0x6924:
case 0x6925:
case 0x6926:
case 0x6927:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6928:
case 0x6929:
case 0x692a:
case 0x692b:
case 0x692c:
case 0x692d:
case 0x692e:
case 0x692f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6930:
case 0x6931:
case 0x6932:
case 0x6933:
case 0x6934:
case 0x6935:
case 0x6936:
case 0x6937:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6938:
case 0x6939:
case 0x693a:
case 0x693b:
case 0x693c:
case 0x693d:
case 0x693e:
case 0x693f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6940:
case 0x6941:
case 0x6942:
case 0x6943:
case 0x6944:
case 0x6945:
case 0x6946:
case 0x6947:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6948:
case 0x6949:
case 0x694a:
case 0x694b:
case 0x694c:
case 0x694d:
case 0x694e:
case 0x694f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6950:
case 0x6951:
case 0x6952:
case 0x6953:
case 0x6954:
case 0x6955:
case 0x6956:
case 0x6957:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6958:
case 0x6959:
case 0x695a:
case 0x695b:
case 0x695c:
case 0x695d:
case 0x695e:
case 0x695f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6960:
case 0x6961:
case 0x6962:
case 0x6963:
case 0x6964:
case 0x6965:
case 0x6966:
case 0x6967:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6968:
case 0x6969:
case 0x696a:
case 0x696b:
case 0x696c:
case 0x696d:
case 0x696e:
case 0x696f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6970:
case 0x6971:
case 0x6972:
case 0x6973:
case 0x6974:
case 0x6975:
case 0x6976:
case 0x6977:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6978:
case 0x6979:
case 0x697a:
case 0x697b:
case 0x697c:
case 0x697d:
case 0x697e:
case 0x697f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6980:
case 0x6981:
case 0x6982:
case 0x6983:
case 0x6984:
case 0x6985:
case 0x6986:
case 0x6987:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6988:
case 0x6989:
case 0x698a:
case 0x698b:
case 0x698c:
case 0x698d:
case 0x698e:
case 0x698f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6990:
case 0x6991:
case 0x6992:
case 0x6993:
case 0x6994:
case 0x6995:
case 0x6996:
case 0x6997:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6998:
case 0x6999:
case 0x699a:
case 0x699b:
case 0x699c:
case 0x699d:
case 0x699e:
case 0x699f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x69a0:
case 0x69a1:
case 0x69a2:
case 0x69a3:
case 0x69a4:
case 0x69a5:
case 0x69a6:
case 0x69a7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x69a8:
case 0x69a9:
case 0x69aa:
case 0x69ab:
case 0x69ac:
case 0x69ad:
case 0x69ae:
case 0x69af:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x69b0:
case 0x69b1:
case 0x69b2:
case 0x69b3:
case 0x69b4:
case 0x69b5:
case 0x69b6:
case 0x69b7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x69b8:
case 0x69b9:
case 0x69ba:
case 0x69bb:
case 0x69bc:
case 0x69bd:
case 0x69be:
case 0x69bf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a00:
case 0x6a01:
case 0x6a02:
case 0x6a03:
case 0x6a04:
case 0x6a05:
case 0x6a06:
case 0x6a07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a08:
case 0x6a09:
case 0x6a0a:
case 0x6a0b:
case 0x6a0c:
case 0x6a0d:
case 0x6a0e:
case 0x6a0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a10:
case 0x6a11:
case 0x6a12:
case 0x6a13:
case 0x6a14:
case 0x6a15:
case 0x6a16:
case 0x6a17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a18:
case 0x6a19:
case 0x6a1a:
case 0x6a1b:
case 0x6a1c:
case 0x6a1d:
case 0x6a1e:
case 0x6a1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a20:
case 0x6a21:
case 0x6a22:
case 0x6a23:
case 0x6a24:
case 0x6a25:
case 0x6a26:
case 0x6a27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a28:
case 0x6a29:
case 0x6a2a:
case 0x6a2b:
case 0x6a2c:
case 0x6a2d:
case 0x6a2e:
case 0x6a2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a30:
case 0x6a31:
case 0x6a32:
case 0x6a33:
case 0x6a34:
case 0x6a35:
case 0x6a36:
case 0x6a37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a38:
case 0x6a39:
case 0x6a3a:
case 0x6a3b:
case 0x6a3c:
case 0x6a3d:
case 0x6a3e:
case 0x6a3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a40:
case 0x6a41:
case 0x6a42:
case 0x6a43:
case 0x6a44:
case 0x6a45:
case 0x6a46:
case 0x6a47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a48:
case 0x6a49:
case 0x6a4a:
case 0x6a4b:
case 0x6a4c:
case 0x6a4d:
case 0x6a4e:
case 0x6a4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a50:
case 0x6a51:
case 0x6a52:
case 0x6a53:
case 0x6a54:
case 0x6a55:
case 0x6a56:
case 0x6a57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a58:
case 0x6a59:
case 0x6a5a:
case 0x6a5b:
case 0x6a5c:
case 0x6a5d:
case 0x6a5e:
case 0x6a5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a60:
case 0x6a61:
case 0x6a62:
case 0x6a63:
case 0x6a64:
case 0x6a65:
case 0x6a66:
case 0x6a67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a68:
case 0x6a69:
case 0x6a6a:
case 0x6a6b:
case 0x6a6c:
case 0x6a6d:
case 0x6a6e:
case 0x6a6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a70:
case 0x6a71:
case 0x6a72:
case 0x6a73:
case 0x6a74:
case 0x6a75:
case 0x6a76:
case 0x6a77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a78:
case 0x6a79:
case 0x6a7a:
case 0x6a7b:
case 0x6a7c:
case 0x6a7d:
case 0x6a7e:
case 0x6a7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6a80:
case 0x6a81:
case 0x6a82:
case 0x6a83:
case 0x6a84:
case 0x6a85:
case 0x6a86:
case 0x6a87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a88:
case 0x6a89:
case 0x6a8a:
case 0x6a8b:
case 0x6a8c:
case 0x6a8d:
case 0x6a8e:
case 0x6a8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a90:
case 0x6a91:
case 0x6a92:
case 0x6a93:
case 0x6a94:
case 0x6a95:
case 0x6a96:
case 0x6a97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6a98:
case 0x6a99:
case 0x6a9a:
case 0x6a9b:
case 0x6a9c:
case 0x6a9d:
case 0x6a9e:
case 0x6a9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6aa0:
case 0x6aa1:
case 0x6aa2:
case 0x6aa3:
case 0x6aa4:
case 0x6aa5:
case 0x6aa6:
case 0x6aa7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6aa8:
case 0x6aa9:
case 0x6aaa:
case 0x6aab:
case 0x6aac:
case 0x6aad:
case 0x6aae:
case 0x6aaf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6ab0:
case 0x6ab1:
case 0x6ab2:
case 0x6ab3:
case 0x6ab4:
case 0x6ab5:
case 0x6ab6:
case 0x6ab7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6ab8:
case 0x6ab9:
case 0x6aba:
case 0x6abb:
case 0x6abc:
case 0x6abd:
case 0x6abe:
case 0x6abf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b00:
case 0x6b01:
case 0x6b02:
case 0x6b03:
case 0x6b04:
case 0x6b05:
case 0x6b06:
case 0x6b07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b08:
case 0x6b09:
case 0x6b0a:
case 0x6b0b:
case 0x6b0c:
case 0x6b0d:
case 0x6b0e:
case 0x6b0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b10:
case 0x6b11:
case 0x6b12:
case 0x6b13:
case 0x6b14:
case 0x6b15:
case 0x6b16:
case 0x6b17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b18:
case 0x6b19:
case 0x6b1a:
case 0x6b1b:
case 0x6b1c:
case 0x6b1d:
case 0x6b1e:
case 0x6b1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b20:
case 0x6b21:
case 0x6b22:
case 0x6b23:
case 0x6b24:
case 0x6b25:
case 0x6b26:
case 0x6b27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b28:
case 0x6b29:
case 0x6b2a:
case 0x6b2b:
case 0x6b2c:
case 0x6b2d:
case 0x6b2e:
case 0x6b2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b30:
case 0x6b31:
case 0x6b32:
case 0x6b33:
case 0x6b34:
case 0x6b35:
case 0x6b36:
case 0x6b37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b38:
case 0x6b39:
case 0x6b3a:
case 0x6b3b:
case 0x6b3c:
case 0x6b3d:
case 0x6b3e:
case 0x6b3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b40:
case 0x6b41:
case 0x6b42:
case 0x6b43:
case 0x6b44:
case 0x6b45:
case 0x6b46:
case 0x6b47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b48:
case 0x6b49:
case 0x6b4a:
case 0x6b4b:
case 0x6b4c:
case 0x6b4d:
case 0x6b4e:
case 0x6b4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b50:
case 0x6b51:
case 0x6b52:
case 0x6b53:
case 0x6b54:
case 0x6b55:
case 0x6b56:
case 0x6b57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b58:
case 0x6b59:
case 0x6b5a:
case 0x6b5b:
case 0x6b5c:
case 0x6b5d:
case 0x6b5e:
case 0x6b5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b60:
case 0x6b61:
case 0x6b62:
case 0x6b63:
case 0x6b64:
case 0x6b65:
case 0x6b66:
case 0x6b67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b68:
case 0x6b69:
case 0x6b6a:
case 0x6b6b:
case 0x6b6c:
case 0x6b6d:
case 0x6b6e:
case 0x6b6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b70:
case 0x6b71:
case 0x6b72:
case 0x6b73:
case 0x6b74:
case 0x6b75:
case 0x6b76:
case 0x6b77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b78:
case 0x6b79:
case 0x6b7a:
case 0x6b7b:
case 0x6b7c:
case 0x6b7d:
case 0x6b7e:
case 0x6b7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6b80:
case 0x6b81:
case 0x6b82:
case 0x6b83:
case 0x6b84:
case 0x6b85:
case 0x6b86:
case 0x6b87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b88:
case 0x6b89:
case 0x6b8a:
case 0x6b8b:
case 0x6b8c:
case 0x6b8d:
case 0x6b8e:
case 0x6b8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b90:
case 0x6b91:
case 0x6b92:
case 0x6b93:
case 0x6b94:
case 0x6b95:
case 0x6b96:
case 0x6b97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6b98:
case 0x6b99:
case 0x6b9a:
case 0x6b9b:
case 0x6b9c:
case 0x6b9d:
case 0x6b9e:
case 0x6b9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6ba0:
case 0x6ba1:
case 0x6ba2:
case 0x6ba3:
case 0x6ba4:
case 0x6ba5:
case 0x6ba6:
case 0x6ba7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6ba8:
case 0x6ba9:
case 0x6baa:
case 0x6bab:
case 0x6bac:
case 0x6bad:
case 0x6bae:
case 0x6baf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6bb0:
case 0x6bb1:
case 0x6bb2:
case 0x6bb3:
case 0x6bb4:
case 0x6bb5:
case 0x6bb6:
case 0x6bb7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6bb8:
case 0x6bb9:
case 0x6bba:
case 0x6bbb:
case 0x6bbc:
case 0x6bbd:
case 0x6bbe:
case 0x6bbf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c00:
case 0x6c01:
case 0x6c02:
case 0x6c03:
case 0x6c04:
case 0x6c05:
case 0x6c06:
case 0x6c07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c08:
case 0x6c09:
case 0x6c0a:
case 0x6c0b:
case 0x6c0c:
case 0x6c0d:
case 0x6c0e:
case 0x6c0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c10:
case 0x6c11:
case 0x6c12:
case 0x6c13:
case 0x6c14:
case 0x6c15:
case 0x6c16:
case 0x6c17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c18:
case 0x6c19:
case 0x6c1a:
case 0x6c1b:
case 0x6c1c:
case 0x6c1d:
case 0x6c1e:
case 0x6c1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c20:
case 0x6c21:
case 0x6c22:
case 0x6c23:
case 0x6c24:
case 0x6c25:
case 0x6c26:
case 0x6c27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c28:
case 0x6c29:
case 0x6c2a:
case 0x6c2b:
case 0x6c2c:
case 0x6c2d:
case 0x6c2e:
case 0x6c2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c30:
case 0x6c31:
case 0x6c32:
case 0x6c33:
case 0x6c34:
case 0x6c35:
case 0x6c36:
case 0x6c37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c38:
case 0x6c39:
case 0x6c3a:
case 0x6c3b:
case 0x6c3c:
case 0x6c3d:
case 0x6c3e:
case 0x6c3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c40:
case 0x6c41:
case 0x6c42:
case 0x6c43:
case 0x6c44:
case 0x6c45:
case 0x6c46:
case 0x6c47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c48:
case 0x6c49:
case 0x6c4a:
case 0x6c4b:
case 0x6c4c:
case 0x6c4d:
case 0x6c4e:
case 0x6c4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c50:
case 0x6c51:
case 0x6c52:
case 0x6c53:
case 0x6c54:
case 0x6c55:
case 0x6c56:
case 0x6c57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c58:
case 0x6c59:
case 0x6c5a:
case 0x6c5b:
case 0x6c5c:
case 0x6c5d:
case 0x6c5e:
case 0x6c5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c60:
case 0x6c61:
case 0x6c62:
case 0x6c63:
case 0x6c64:
case 0x6c65:
case 0x6c66:
case 0x6c67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c68:
case 0x6c69:
case 0x6c6a:
case 0x6c6b:
case 0x6c6c:
case 0x6c6d:
case 0x6c6e:
case 0x6c6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c70:
case 0x6c71:
case 0x6c72:
case 0x6c73:
case 0x6c74:
case 0x6c75:
case 0x6c76:
case 0x6c77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c78:
case 0x6c79:
case 0x6c7a:
case 0x6c7b:
case 0x6c7c:
case 0x6c7d:
case 0x6c7e:
case 0x6c7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6c80:
case 0x6c81:
case 0x6c82:
case 0x6c83:
case 0x6c84:
case 0x6c85:
case 0x6c86:
case 0x6c87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c88:
case 0x6c89:
case 0x6c8a:
case 0x6c8b:
case 0x6c8c:
case 0x6c8d:
case 0x6c8e:
case 0x6c8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c90:
case 0x6c91:
case 0x6c92:
case 0x6c93:
case 0x6c94:
case 0x6c95:
case 0x6c96:
case 0x6c97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6c98:
case 0x6c99:
case 0x6c9a:
case 0x6c9b:
case 0x6c9c:
case 0x6c9d:
case 0x6c9e:
case 0x6c9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6ca0:
case 0x6ca1:
case 0x6ca2:
case 0x6ca3:
case 0x6ca4:
case 0x6ca5:
case 0x6ca6:
case 0x6ca7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6ca8:
case 0x6ca9:
case 0x6caa:
case 0x6cab:
case 0x6cac:
case 0x6cad:
case 0x6cae:
case 0x6caf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6cb0:
case 0x6cb1:
case 0x6cb2:
case 0x6cb3:
case 0x6cb4:
case 0x6cb5:
case 0x6cb6:
case 0x6cb7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6cb8:
case 0x6cb9:
case 0x6cba:
case 0x6cbb:
case 0x6cbc:
case 0x6cbd:
case 0x6cbe:
case 0x6cbf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d00:
case 0x6d01:
case 0x6d02:
case 0x6d03:
case 0x6d04:
case 0x6d05:
case 0x6d06:
case 0x6d07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d08:
case 0x6d09:
case 0x6d0a:
case 0x6d0b:
case 0x6d0c:
case 0x6d0d:
case 0x6d0e:
case 0x6d0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d10:
case 0x6d11:
case 0x6d12:
case 0x6d13:
case 0x6d14:
case 0x6d15:
case 0x6d16:
case 0x6d17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d18:
case 0x6d19:
case 0x6d1a:
case 0x6d1b:
case 0x6d1c:
case 0x6d1d:
case 0x6d1e:
case 0x6d1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d20:
case 0x6d21:
case 0x6d22:
case 0x6d23:
case 0x6d24:
case 0x6d25:
case 0x6d26:
case 0x6d27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d28:
case 0x6d29:
case 0x6d2a:
case 0x6d2b:
case 0x6d2c:
case 0x6d2d:
case 0x6d2e:
case 0x6d2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d30:
case 0x6d31:
case 0x6d32:
case 0x6d33:
case 0x6d34:
case 0x6d35:
case 0x6d36:
case 0x6d37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d38:
case 0x6d39:
case 0x6d3a:
case 0x6d3b:
case 0x6d3c:
case 0x6d3d:
case 0x6d3e:
case 0x6d3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d40:
case 0x6d41:
case 0x6d42:
case 0x6d43:
case 0x6d44:
case 0x6d45:
case 0x6d46:
case 0x6d47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d48:
case 0x6d49:
case 0x6d4a:
case 0x6d4b:
case 0x6d4c:
case 0x6d4d:
case 0x6d4e:
case 0x6d4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d50:
case 0x6d51:
case 0x6d52:
case 0x6d53:
case 0x6d54:
case 0x6d55:
case 0x6d56:
case 0x6d57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d58:
case 0x6d59:
case 0x6d5a:
case 0x6d5b:
case 0x6d5c:
case 0x6d5d:
case 0x6d5e:
case 0x6d5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d60:
case 0x6d61:
case 0x6d62:
case 0x6d63:
case 0x6d64:
case 0x6d65:
case 0x6d66:
case 0x6d67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d68:
case 0x6d69:
case 0x6d6a:
case 0x6d6b:
case 0x6d6c:
case 0x6d6d:
case 0x6d6e:
case 0x6d6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d70:
case 0x6d71:
case 0x6d72:
case 0x6d73:
case 0x6d74:
case 0x6d75:
case 0x6d76:
case 0x6d77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d78:
case 0x6d79:
case 0x6d7a:
case 0x6d7b:
case 0x6d7c:
case 0x6d7d:
case 0x6d7e:
case 0x6d7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6d80:
case 0x6d81:
case 0x6d82:
case 0x6d83:
case 0x6d84:
case 0x6d85:
case 0x6d86:
case 0x6d87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d88:
case 0x6d89:
case 0x6d8a:
case 0x6d8b:
case 0x6d8c:
case 0x6d8d:
case 0x6d8e:
case 0x6d8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d90:
case 0x6d91:
case 0x6d92:
case 0x6d93:
case 0x6d94:
case 0x6d95:
case 0x6d96:
case 0x6d97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6d98:
case 0x6d99:
case 0x6d9a:
case 0x6d9b:
case 0x6d9c:
case 0x6d9d:
case 0x6d9e:
case 0x6d9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6da0:
case 0x6da1:
case 0x6da2:
case 0x6da3:
case 0x6da4:
case 0x6da5:
case 0x6da6:
case 0x6da7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6da8:
case 0x6da9:
case 0x6daa:
case 0x6dab:
case 0x6dac:
case 0x6dad:
case 0x6dae:
case 0x6daf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6db0:
case 0x6db1:
case 0x6db2:
case 0x6db3:
case 0x6db4:
case 0x6db5:
case 0x6db6:
case 0x6db7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6db8:
case 0x6db9:
case 0x6dba:
case 0x6dbb:
case 0x6dbc:
case 0x6dbd:
case 0x6dbe:
case 0x6dbf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e00:
case 0x6e01:
case 0x6e02:
case 0x6e03:
case 0x6e04:
case 0x6e05:
case 0x6e06:
case 0x6e07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e08:
case 0x6e09:
case 0x6e0a:
case 0x6e0b:
case 0x6e0c:
case 0x6e0d:
case 0x6e0e:
case 0x6e0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e10:
case 0x6e11:
case 0x6e12:
case 0x6e13:
case 0x6e14:
case 0x6e15:
case 0x6e16:
case 0x6e17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e18:
case 0x6e19:
case 0x6e1a:
case 0x6e1b:
case 0x6e1c:
case 0x6e1d:
case 0x6e1e:
case 0x6e1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e20:
case 0x6e21:
case 0x6e22:
case 0x6e23:
case 0x6e24:
case 0x6e25:
case 0x6e26:
case 0x6e27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e28:
case 0x6e29:
case 0x6e2a:
case 0x6e2b:
case 0x6e2c:
case 0x6e2d:
case 0x6e2e:
case 0x6e2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e30:
case 0x6e31:
case 0x6e32:
case 0x6e33:
case 0x6e34:
case 0x6e35:
case 0x6e36:
case 0x6e37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e38:
case 0x6e39:
case 0x6e3a:
case 0x6e3b:
case 0x6e3c:
case 0x6e3d:
case 0x6e3e:
case 0x6e3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e40:
case 0x6e41:
case 0x6e42:
case 0x6e43:
case 0x6e44:
case 0x6e45:
case 0x6e46:
case 0x6e47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e48:
case 0x6e49:
case 0x6e4a:
case 0x6e4b:
case 0x6e4c:
case 0x6e4d:
case 0x6e4e:
case 0x6e4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e50:
case 0x6e51:
case 0x6e52:
case 0x6e53:
case 0x6e54:
case 0x6e55:
case 0x6e56:
case 0x6e57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e58:
case 0x6e59:
case 0x6e5a:
case 0x6e5b:
case 0x6e5c:
case 0x6e5d:
case 0x6e5e:
case 0x6e5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e60:
case 0x6e61:
case 0x6e62:
case 0x6e63:
case 0x6e64:
case 0x6e65:
case 0x6e66:
case 0x6e67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e68:
case 0x6e69:
case 0x6e6a:
case 0x6e6b:
case 0x6e6c:
case 0x6e6d:
case 0x6e6e:
case 0x6e6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e70:
case 0x6e71:
case 0x6e72:
case 0x6e73:
case 0x6e74:
case 0x6e75:
case 0x6e76:
case 0x6e77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e78:
case 0x6e79:
case 0x6e7a:
case 0x6e7b:
case 0x6e7c:
case 0x6e7d:
case 0x6e7e:
case 0x6e7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6e80:
case 0x6e81:
case 0x6e82:
case 0x6e83:
case 0x6e84:
case 0x6e85:
case 0x6e86:
case 0x6e87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e88:
case 0x6e89:
case 0x6e8a:
case 0x6e8b:
case 0x6e8c:
case 0x6e8d:
case 0x6e8e:
case 0x6e8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e90:
case 0x6e91:
case 0x6e92:
case 0x6e93:
case 0x6e94:
case 0x6e95:
case 0x6e96:
case 0x6e97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6e98:
case 0x6e99:
case 0x6e9a:
case 0x6e9b:
case 0x6e9c:
case 0x6e9d:
case 0x6e9e:
case 0x6e9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6ea0:
case 0x6ea1:
case 0x6ea2:
case 0x6ea3:
case 0x6ea4:
case 0x6ea5:
case 0x6ea6:
case 0x6ea7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6ea8:
case 0x6ea9:
case 0x6eaa:
case 0x6eab:
case 0x6eac:
case 0x6ead:
case 0x6eae:
case 0x6eaf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6eb0:
case 0x6eb1:
case 0x6eb2:
case 0x6eb3:
case 0x6eb4:
case 0x6eb5:
case 0x6eb6:
case 0x6eb7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6eb8:
case 0x6eb9:
case 0x6eba:
case 0x6ebb:
case 0x6ebc:
case 0x6ebd:
case 0x6ebe:
case 0x6ebf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROR(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f00:
case 0x6f01:
case 0x6f02:
case 0x6f03:
case 0x6f04:
case 0x6f05:
case 0x6f06:
case 0x6f07:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f08:
case 0x6f09:
case 0x6f0a:
case 0x6f0b:
case 0x6f0c:
case 0x6f0d:
case 0x6f0e:
case 0x6f0f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f10:
case 0x6f11:
case 0x6f12:
case 0x6f13:
case 0x6f14:
case 0x6f15:
case 0x6f16:
case 0x6f17:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f18:
case 0x6f19:
case 0x6f1a:
case 0x6f1b:
case 0x6f1c:
case 0x6f1d:
case 0x6f1e:
case 0x6f1f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f20:
case 0x6f21:
case 0x6f22:
case 0x6f23:
case 0x6f24:
case 0x6f25:
case 0x6f26:
case 0x6f27:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f28:
case 0x6f29:
case 0x6f2a:
case 0x6f2b:
case 0x6f2c:
case 0x6f2d:
case 0x6f2e:
case 0x6f2f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f30:
case 0x6f31:
case 0x6f32:
case 0x6f33:
case 0x6f34:
case 0x6f35:
case 0x6f36:
case 0x6f37:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f38:
case 0x6f39:
case 0x6f3a:
case 0x6f3b:
case 0x6f3c:
case 0x6f3d:
case 0x6f3e:
case 0x6f3f:
	{
	 HAM<uint8, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f40:
case 0x6f41:
case 0x6f42:
case 0x6f43:
case 0x6f44:
case 0x6f45:
case 0x6f46:
case 0x6f47:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f48:
case 0x6f49:
case 0x6f4a:
case 0x6f4b:
case 0x6f4c:
case 0x6f4d:
case 0x6f4e:
case 0x6f4f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f50:
case 0x6f51:
case 0x6f52:
case 0x6f53:
case 0x6f54:
case 0x6f55:
case 0x6f56:
case 0x6f57:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f58:
case 0x6f59:
case 0x6f5a:
case 0x6f5b:
case 0x6f5c:
case 0x6f5d:
case 0x6f5e:
case 0x6f5f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f60:
case 0x6f61:
case 0x6f62:
case 0x6f63:
case 0x6f64:
case 0x6f65:
case 0x6f66:
case 0x6f67:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f68:
case 0x6f69:
case 0x6f6a:
case 0x6f6b:
case 0x6f6c:
case 0x6f6d:
case 0x6f6e:
case 0x6f6f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f70:
case 0x6f71:
case 0x6f72:
case 0x6f73:
case 0x6f74:
case 0x6f75:
case 0x6f76:
case 0x6f77:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f78:
case 0x6f79:
case 0x6f7a:
case 0x6f7b:
case 0x6f7c:
case 0x6f7d:
case 0x6f7e:
case 0x6f7f:
	{
	 HAM<uint16, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6f80:
case 0x6f81:
case 0x6f82:
case 0x6f83:
case 0x6f84:
case 0x6f85:
case 0x6f86:
case 0x6f87:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f88:
case 0x6f89:
case 0x6f8a:
case 0x6f8b:
case 0x6f8c:
case 0x6f8d:
case 0x6f8e:
case 0x6f8f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f90:
case 0x6f91:
case 0x6f92:
case 0x6f93:
case 0x6f94:
case 0x6f95:
case 0x6f96:
case 0x6f97:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6f98:
case 0x6f99:
case 0x6f9a:
case 0x6f9b:
case 0x6f9c:
case 0x6f9d:
case 0x6f9e:
case 0x6f9f:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, instr_b11_b9 ? instr_b11_b9 : 8);
	}
	break;

case 0x6fa0:
case 0x6fa1:
case 0x6fa2:
case 0x6fa3:
case 0x6fa4:
case 0x6fa5:
case 0x6fa6:
case 0x6fa7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ASL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6fa8:
case 0x6fa9:
case 0x6faa:
case 0x6fab:
case 0x6fac:
case 0x6fad:
case 0x6fae:
case 0x6faf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); LSL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6fb0:
case 0x6fb1:
case 0x6fb2:
case 0x6fb3:
case 0x6fb4:
case 0x6fb5:
case 0x6fb6:
case 0x6fb7:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROXL(targ, D[instr_b11_b9]);
	}
	break;

case 0x6fb8:
case 0x6fb9:
case 0x6fba:
case 0x6fbb:
case 0x6fbc:
case 0x6fbd:
case 0x6fbe:
case 0x6fbf:
	{
	 HAM<uint32, DATA_REG_DIR> targ(this, instr_b2_b0); ROL(targ, D[instr_b11_b9]);
	}
	break;

case 0x7000:
case 0x7001:
case 0x7002:
case 0x7003:
case 0x7004:
case 0x7005:
case 0x7006:
case 0x7007:
case 0x7008:
case 0x7009:
case 0x700a:
case 0x700b:
case 0x700c:
case 0x700d:
case 0x700e:
case 0x700f:
case 0x7010:
case 0x7011:
case 0x7012:
case 0x7013:
case 0x7014:
case 0x7015:
case 0x7016:
case 0x7017:
case 0x7018:
case 0x7019:
case 0x701a:
case 0x701b:
case 0x701c:
case 0x701d:
case 0x701e:
case 0x701f:
case 0x7020:
case 0x7021:
case 0x7022:
case 0x7023:
case 0x7024:
case 0x7025:
case 0x7026:
case 0x7027:
case 0x7028:
case 0x7029:
case 0x702a:
case 0x702b:
case 0x702c:
case 0x702d:
case 0x702e:
case 0x702f:
case 0x7030:
case 0x7031:
case 0x7032:
case 0x7033:
case 0x7034:
case 0x7035:
case 0x7036:
case 0x7037:
case 0x7038:
case 0x7039:
case 0x703a:
case 0x703b:
case 0x703c:
case 0x703d:
case 0x703e:
case 0x703f:
case 0x7040:
case 0x7041:
case 0x7042:
case 0x7043:
case 0x7044:
case 0x7045:
case 0x7046:
case 0x7047:
case 0x7048:
case 0x7049:
case 0x704a:
case 0x704b:
case 0x704c:
case 0x704d:
case 0x704e:
case 0x704f:
case 0x7050:
case 0x7051:
case 0x7052:
case 0x7053:
case 0x7054:
case 0x7055:
case 0x7056:
case 0x7057:
case 0x7058:
case 0x7059:
case 0x705a:
case 0x705b:
case 0x705c:
case 0x705d:
case 0x705e:
case 0x705f:
case 0x7060:
case 0x7061:
case 0x7062:
case 0x7063:
case 0x7064:
case 0x7065:
case 0x7066:
case 0x7067:
case 0x7068:
case 0x7069:
case 0x706a:
case 0x706b:
case 0x706c:
case 0x706d:
case 0x706e:
case 0x706f:
case 0x7070:
case 0x7071:
case 0x7072:
case 0x7073:
case 0x7074:
case 0x7075:
case 0x7076:
case 0x7077:
case 0x7078:
case 0x7079:
case 0x707a:
case 0x707b:
case 0x707c:
case 0x707d:
case 0x707e:
case 0x707f:
case 0x7080:
case 0x7081:
case 0x7082:
case 0x7083:
case 0x7084:
case 0x7085:
case 0x7086:
case 0x7087:
case 0x7088:
case 0x7089:
case 0x708a:
case 0x708b:
case 0x708c:
case 0x708d:
case 0x708e:
case 0x708f:
case 0x7090:
case 0x7091:
case 0x7092:
case 0x7093:
case 0x7094:
case 0x7095:
case 0x7096:
case 0x7097:
case 0x7098:
case 0x7099:
case 0x709a:
case 0x709b:
case 0x709c:
case 0x709d:
case 0x709e:
case 0x709f:
case 0x70a0:
case 0x70a1:
case 0x70a2:
case 0x70a3:
case 0x70a4:
case 0x70a5:
case 0x70a6:
case 0x70a7:
case 0x70a8:
case 0x70a9:
case 0x70aa:
case 0x70ab:
case 0x70ac:
case 0x70ad:
case 0x70ae:
case 0x70af:
case 0x70b0:
case 0x70b1:
case 0x70b2:
case 0x70b3:
case 0x70b4:
case 0x70b5:
case 0x70b6:
case 0x70b7:
case 0x70b8:
case 0x70b9:
case 0x70ba:
case 0x70bb:
case 0x70bc:
case 0x70bd:
case 0x70be:
case 0x70bf:
case 0x70c0:
case 0x70c1:
case 0x70c2:
case 0x70c3:
case 0x70c4:
case 0x70c5:
case 0x70c6:
case 0x70c7:
case 0x70c8:
case 0x70c9:
case 0x70ca:
case 0x70cb:
case 0x70cc:
case 0x70cd:
case 0x70ce:
case 0x70cf:
case 0x70d0:
case 0x70d1:
case 0x70d2:
case 0x70d3:
case 0x70d4:
case 0x70d5:
case 0x70d6:
case 0x70d7:
case 0x70d8:
case 0x70d9:
case 0x70da:
case 0x70db:
case 0x70dc:
case 0x70dd:
case 0x70de:
case 0x70df:
case 0x70e0:
case 0x70e1:
case 0x70e2:
case 0x70e3:
case 0x70e4:
case 0x70e5:
case 0x70e6:
case 0x70e7:
case 0x70e8:
case 0x70e9:
case 0x70ea:
case 0x70eb:
case 0x70ec:
case 0x70ed:
case 0x70ee:
case 0x70ef:
case 0x70f0:
case 0x70f1:
case 0x70f2:
case 0x70f3:
case 0x70f4:
case 0x70f5:
case 0x70f6:
case 0x70f7:
case 0x70f8:
case 0x70f9:
case 0x70fa:
case 0x70fb:
case 0x70fc:
case 0x70fd:
case 0x70fe:
case 0x70ff:
case 0x7100:
case 0x7101:
case 0x7102:
case 0x7103:
case 0x7104:
case 0x7105:
case 0x7106:
case 0x7107:
case 0x7108:
case 0x7109:
case 0x710a:
case 0x710b:
case 0x710c:
case 0x710d:
case 0x710e:
case 0x710f:
case 0x7110:
case 0x7111:
case 0x7112:
case 0x7113:
case 0x7114:
case 0x7115:
case 0x7116:
case 0x7117:
case 0x7118:
case 0x7119:
case 0x711a:
case 0x711b:
case 0x711c:
case 0x711d:
case 0x711e:
case 0x711f:
case 0x7120:
case 0x7121:
case 0x7122:
case 0x7123:
case 0x7124:
case 0x7125:
case 0x7126:
case 0x7127:
case 0x7128:
case 0x7129:
case 0x712a:
case 0x712b:
case 0x712c:
case 0x712d:
case 0x712e:
case 0x712f:
case 0x7130:
case 0x7131:
case 0x7132:
case 0x7133:
case 0x7134:
case 0x7135:
case 0x7136:
case 0x7137:
case 0x7138:
case 0x7139:
case 0x713a:
case 0x713b:
case 0x713c:
case 0x713d:
case 0x713e:
case 0x713f:
case 0x7140:
case 0x7141:
case 0x7142:
case 0x7143:
case 0x7144:
case 0x7145:
case 0x7146:
case 0x7147:
case 0x7148:
case 0x7149:
case 0x714a:
case 0x714b:
case 0x714c:
case 0x714d:
case 0x714e:
case 0x714f:
case 0x7150:
case 0x7151:
case 0x7152:
case 0x7153:
case 0x7154:
case 0x7155:
case 0x7156:
case 0x7157:
case 0x7158:
case 0x7159:
case 0x715a:
case 0x715b:
case 0x715c:
case 0x715d:
case 0x715e:
case 0x715f:
case 0x7160:
case 0x7161:
case 0x7162:
case 0x7163:
case 0x7164:
case 0x7165:
case 0x7166:
case 0x7167:
case 0x7168:
case 0x7169:
case 0x716a:
case 0x716b:
case 0x716c:
case 0x716d:
case 0x716e:
case 0x716f:
case 0x7170:
case 0x7171:
case 0x7172:
case 0x7173:
case 0x7174:
case 0x7175:
case 0x7176:
case 0x7177:
case 0x7178:
case 0x7179:
case 0x717a:
case 0x717b:
case 0x717c:
case 0x717d:
case 0x717e:
case 0x717f:
case 0x7180:
case 0x7181:
case 0x7182:
case 0x7183:
case 0x7184:
case 0x7185:
case 0x7186:
case 0x7187:
case 0x7188:
case 0x7189:
case 0x718a:
case 0x718b:
case 0x718c:
case 0x718d:
case 0x718e:
case 0x718f:
case 0x7190:
case 0x7191:
case 0x7192:
case 0x7193:
case 0x7194:
case 0x7195:
case 0x7196:
case 0x7197:
case 0x7198:
case 0x7199:
case 0x719a:
case 0x719b:
case 0x719c:
case 0x719d:
case 0x719e:
case 0x719f:
case 0x71a0:
case 0x71a1:
case 0x71a2:
case 0x71a3:
case 0x71a4:
case 0x71a5:
case 0x71a6:
case 0x71a7:
case 0x71a8:
case 0x71a9:
case 0x71aa:
case 0x71ab:
case 0x71ac:
case 0x71ad:
case 0x71ae:
case 0x71af:
case 0x71b0:
case 0x71b1:
case 0x71b2:
case 0x71b3:
case 0x71b4:
case 0x71b5:
case 0x71b6:
case 0x71b7:
case 0x71b8:
case 0x71b9:
case 0x71ba:
case 0x71bb:
case 0x71bc:
case 0x71bd:
case 0x71be:
case 0x71bf:
case 0x71c0:
case 0x71c1:
case 0x71c2:
case 0x71c3:
case 0x71c4:
case 0x71c5:
case 0x71c6:
case 0x71c7:
case 0x71c8:
case 0x71c9:
case 0x71ca:
case 0x71cb:
case 0x71cc:
case 0x71cd:
case 0x71ce:
case 0x71cf:
case 0x71d0:
case 0x71d1:
case 0x71d2:
case 0x71d3:
case 0x71d4:
case 0x71d5:
case 0x71d6:
case 0x71d7:
case 0x71d8:
case 0x71d9:
case 0x71da:
case 0x71db:
case 0x71dc:
case 0x71dd:
case 0x71de:
case 0x71df:
case 0x71e0:
case 0x71e1:
case 0x71e2:
case 0x71e3:
case 0x71e4:
case 0x71e5:
case 0x71e6:
case 0x71e7:
case 0x71e8:
case 0x71e9:
case 0x71ea:
case 0x71eb:
case 0x71ec:
case 0x71ed:
case 0x71ee:
case 0x71ef:
case 0x71f0:
case 0x71f1:
case 0x71f2:
case 0x71f3:
case 0x71f4:
case 0x71f5:
case 0x71f6:
case 0x71f7:
case 0x71f8:
case 0x71f9:
case 0x71fa:
case 0x71fb:
case 0x71fc:
case 0x71fd:
case 0x71fe:
case 0x71ff:
case 0x7200:
case 0x7201:
case 0x7202:
case 0x7203:
case 0x7204:
case 0x7205:
case 0x7206:
case 0x7207:
case 0x7208:
case 0x7209:
case 0x720a:
case 0x720b:
case 0x720c:
case 0x720d:
case 0x720e:
case 0x720f:
case 0x7210:
case 0x7211:
case 0x7212:
case 0x7213:
case 0x7214:
case 0x7215:
case 0x7216:
case 0x7217:
case 0x7218:
case 0x7219:
case 0x721a:
case 0x721b:
case 0x721c:
case 0x721d:
case 0x721e:
case 0x721f:
case 0x7220:
case 0x7221:
case 0x7222:
case 0x7223:
case 0x7224:
case 0x7225:
case 0x7226:
case 0x7227:
case 0x7228:
case 0x7229:
case 0x722a:
case 0x722b:
case 0x722c:
case 0x722d:
case 0x722e:
case 0x722f:
case 0x7230:
case 0x7231:
case 0x7232:
case 0x7233:
case 0x7234:
case 0x7235:
case 0x7236:
case 0x7237:
case 0x7238:
case 0x7239:
case 0x723a:
case 0x723b:
case 0x723c:
case 0x723d:
case 0x723e:
case 0x723f:
case 0x7240:
case 0x7241:
case 0x7242:
case 0x7243:
case 0x7244:
case 0x7245:
case 0x7246:
case 0x7247:
case 0x7248:
case 0x7249:
case 0x724a:
case 0x724b:
case 0x724c:
case 0x724d:
case 0x724e:
case 0x724f:
case 0x7250:
case 0x7251:
case 0x7252:
case 0x7253:
case 0x7254:
case 0x7255:
case 0x7256:
case 0x7257:
case 0x7258:
case 0x7259:
case 0x725a:
case 0x725b:
case 0x725c:
case 0x725d:
case 0x725e:
case 0x725f:
case 0x7260:
case 0x7261:
case 0x7262:
case 0x7263:
case 0x7264:
case 0x7265:
case 0x7266:
case 0x7267:
case 0x7268:
case 0x7269:
case 0x726a:
case 0x726b:
case 0x726c:
case 0x726d:
case 0x726e:
case 0x726f:
case 0x7270:
case 0x7271:
case 0x7272:
case 0x7273:
case 0x7274:
case 0x7275:
case 0x7276:
case 0x7277:
case 0x7278:
case 0x7279:
case 0x727a:
case 0x727b:
case 0x727c:
case 0x727d:
case 0x727e:
case 0x727f:
case 0x7280:
case 0x7281:
case 0x7282:
case 0x7283:
case 0x7284:
case 0x7285:
case 0x7286:
case 0x7287:
case 0x7288:
case 0x7289:
case 0x728a:
case 0x728b:
case 0x728c:
case 0x728d:
case 0x728e:
case 0x728f:
case 0x7290:
case 0x7291:
case 0x7292:
case 0x7293:
case 0x7294:
case 0x7295:
case 0x7296:
case 0x7297:
case 0x7298:
case 0x7299:
case 0x729a:
case 0x729b:
case 0x729c:
case 0x729d:
case 0x729e:
case 0x729f:
case 0x72a0:
case 0x72a1:
case 0x72a2:
case 0x72a3:
case 0x72a4:
case 0x72a5:
case 0x72a6:
case 0x72a7:
case 0x72a8:
case 0x72a9:
case 0x72aa:
case 0x72ab:
case 0x72ac:
case 0x72ad:
case 0x72ae:
case 0x72af:
case 0x72b0:
case 0x72b1:
case 0x72b2:
case 0x72b3:
case 0x72b4:
case 0x72b5:
case 0x72b6:
case 0x72b7:
case 0x72b8:
case 0x72b9:
case 0x72ba:
case 0x72bb:
case 0x72bc:
case 0x72bd:
case 0x72be:
case 0x72bf:
case 0x72c0:
case 0x72c1:
case 0x72c2:
case 0x72c3:
case 0x72c4:
case 0x72c5:
case 0x72c6:
case 0x72c7:
case 0x72c8:
case 0x72c9:
case 0x72ca:
case 0x72cb:
case 0x72cc:
case 0x72cd:
case 0x72ce:
case 0x72cf:
case 0x72d0:
case 0x72d1:
case 0x72d2:
case 0x72d3:
case 0x72d4:
case 0x72d5:
case 0x72d6:
case 0x72d7:
case 0x72d8:
case 0x72d9:
case 0x72da:
case 0x72db:
case 0x72dc:
case 0x72dd:
case 0x72de:
case 0x72df:
case 0x72e0:
case 0x72e1:
case 0x72e2:
case 0x72e3:
case 0x72e4:
case 0x72e5:
case 0x72e6:
case 0x72e7:
case 0x72e8:
case 0x72e9:
case 0x72ea:
case 0x72eb:
case 0x72ec:
case 0x72ed:
case 0x72ee:
case 0x72ef:
case 0x72f0:
case 0x72f1:
case 0x72f2:
case 0x72f3:
case 0x72f4:
case 0x72f5:
case 0x72f6:
case 0x72f7:
case 0x72f8:
case 0x72f9:
case 0x72fa:
case 0x72fb:
case 0x72fc:
case 0x72fd:
case 0x72fe:
case 0x72ff:
case 0x7300:
case 0x7301:
case 0x7302:
case 0x7303:
case 0x7304:
case 0x7305:
case 0x7306:
case 0x7307:
case 0x7308:
case 0x7309:
case 0x730a:
case 0x730b:
case 0x730c:
case 0x730d:
case 0x730e:
case 0x730f:
case 0x7310:
case 0x7311:
case 0x7312:
case 0x7313:
case 0x7314:
case 0x7315:
case 0x7316:
case 0x7317:
case 0x7318:
case 0x7319:
case 0x731a:
case 0x731b:
case 0x731c:
case 0x731d:
case 0x731e:
case 0x731f:
case 0x7320:
case 0x7321:
case 0x7322:
case 0x7323:
case 0x7324:
case 0x7325:
case 0x7326:
case 0x7327:
case 0x7328:
case 0x7329:
case 0x732a:
case 0x732b:
case 0x732c:
case 0x732d:
case 0x732e:
case 0x732f:
case 0x7330:
case 0x7331:
case 0x7332:
case 0x7333:
case 0x7334:
case 0x7335:
case 0x7336:
case 0x7337:
case 0x7338:
case 0x7339:
case 0x733a:
case 0x733b:
case 0x733c:
case 0x733d:
case 0x733e:
case 0x733f:
case 0x7340:
case 0x7341:
case 0x7342:
case 0x7343:
case 0x7344:
case 0x7345:
case 0x7346:
case 0x7347:
case 0x7348:
case 0x7349:
case 0x734a:
case 0x734b:
case 0x734c:
case 0x734d:
case 0x734e:
case 0x734f:
case 0x7350:
case 0x7351:
case 0x7352:
case 0x7353:
case 0x7354:
case 0x7355:
case 0x7356:
case 0x7357:
case 0x7358:
case 0x7359:
case 0x735a:
case 0x735b:
case 0x735c:
case 0x735d:
case 0x735e:
case 0x735f:
case 0x7360:
case 0x7361:
case 0x7362:
case 0x7363:
case 0x7364:
case 0x7365:
case 0x7366:
case 0x7367:
case 0x7368:
case 0x7369:
case 0x736a:
case 0x736b:
case 0x736c:
case 0x736d:
case 0x736e:
case 0x736f:
case 0x7370:
case 0x7371:
case 0x7372:
case 0x7373:
case 0x7374:
case 0x7375:
case 0x7376:
case 0x7377:
case 0x7378:
case 0x7379:
case 0x737a:
case 0x737b:
case 0x737c:
case 0x737d:
case 0x737e:
case 0x737f:
case 0x7380:
case 0x7381:
case 0x7382:
case 0x7383:
case 0x7384:
case 0x7385:
case 0x7386:
case 0x7387:
case 0x7388:
case 0x7389:
case 0x738a:
case 0x738b:
case 0x738c:
case 0x738d:
case 0x738e:
case 0x738f:
case 0x7390:
case 0x7391:
case 0x7392:
case 0x7393:
case 0x7394:
case 0x7395:
case 0x7396:
case 0x7397:
case 0x7398:
case 0x7399:
case 0x739a:
case 0x739b:
case 0x739c:
case 0x739d:
case 0x739e:
case 0x739f:
case 0x73a0:
case 0x73a1:
case 0x73a2:
case 0x73a3:
case 0x73a4:
case 0x73a5:
case 0x73a6:
case 0x73a7:
case 0x73a8:
case 0x73a9:
case 0x73aa:
case 0x73ab:
case 0x73ac:
case 0x73ad:
case 0x73ae:
case 0x73af:
case 0x73b0:
case 0x73b1:
case 0x73b2:
case 0x73b3:
case 0x73b4:
case 0x73b5:
case 0x73b6:
case 0x73b7:
case 0x73b8:
case 0x73b9:
case 0x73ba:
case 0x73bb:
case 0x73bc:
case 0x73bd:
case 0x73be:
case 0x73bf:
case 0x73c0:
case 0x73c1:
case 0x73c2:
case 0x73c3:
case 0x73c4:
case 0x73c5:
case 0x73c6:
case 0x73c7:
case 0x73c8:
case 0x73c9:
case 0x73ca:
case 0x73cb:
case 0x73cc:
case 0x73cd:
case 0x73ce:
case 0x73cf:
case 0x73d0:
case 0x73d1:
case 0x73d2:
case 0x73d3:
case 0x73d4:
case 0x73d5:
case 0x73d6:
case 0x73d7:
case 0x73d8:
case 0x73d9:
case 0x73da:
case 0x73db:
case 0x73dc:
case 0x73dd:
case 0x73de:
case 0x73df:
case 0x73e0:
case 0x73e1:
case 0x73e2:
case 0x73e3:
case 0x73e4:
case 0x73e5:
case 0x73e6:
case 0x73e7:
case 0x73e8:
case 0x73e9:
case 0x73ea:
case 0x73eb:
case 0x73ec:
case 0x73ed:
case 0x73ee:
case 0x73ef:
case 0x73f0:
case 0x73f1:
case 0x73f2:
case 0x73f3:
case 0x73f4:
case 0x73f5:
case 0x73f6:
case 0x73f7:
case 0x73f8:
case 0x73f9:
case 0x73fa:
case 0x73fb:
case 0x73fc:
case 0x73fd:
case 0x73fe:
case 0x73ff:
case 0x7400:
case 0x7401:
case 0x7402:
case 0x7403:
case 0x7404:
case 0x7405:
case 0x7406:
case 0x7407:
case 0x7408:
case 0x7409:
case 0x740a:
case 0x740b:
case 0x740c:
case 0x740d:
case 0x740e:
case 0x740f:
case 0x7410:
case 0x7411:
case 0x7412:
case 0x7413:
case 0x7414:
case 0x7415:
case 0x7416:
case 0x7417:
case 0x7418:
case 0x7419:
case 0x741a:
case 0x741b:
case 0x741c:
case 0x741d:
case 0x741e:
case 0x741f:
case 0x7420:
case 0x7421:
case 0x7422:
case 0x7423:
case 0x7424:
case 0x7425:
case 0x7426:
case 0x7427:
case 0x7428:
case 0x7429:
case 0x742a:
case 0x742b:
case 0x742c:
case 0x742d:
case 0x742e:
case 0x742f:
case 0x7430:
case 0x7431:
case 0x7432:
case 0x7433:
case 0x7434:
case 0x7435:
case 0x7436:
case 0x7437:
case 0x7438:
case 0x7439:
case 0x743a:
case 0x743b:
case 0x743c:
case 0x743d:
case 0x743e:
case 0x743f:
case 0x7440:
case 0x7441:
case 0x7442:
case 0x7443:
case 0x7444:
case 0x7445:
case 0x7446:
case 0x7447:
case 0x7448:
case 0x7449:
case 0x744a:
case 0x744b:
case 0x744c:
case 0x744d:
case 0x744e:
case 0x744f:
case 0x7450:
case 0x7451:
case 0x7452:
case 0x7453:
case 0x7454:
case 0x7455:
case 0x7456:
case 0x7457:
case 0x7458:
case 0x7459:
case 0x745a:
case 0x745b:
case 0x745c:
case 0x745d:
case 0x745e:
case 0x745f:
case 0x7460:
case 0x7461:
case 0x7462:
case 0x7463:
case 0x7464:
case 0x7465:
case 0x7466:
case 0x7467:
case 0x7468:
case 0x7469:
case 0x746a:
case 0x746b:
case 0x746c:
case 0x746d:
case 0x746e:
case 0x746f:
case 0x7470:
case 0x7471:
case 0x7472:
case 0x7473:
case 0x7474:
case 0x7475:
case 0x7476:
case 0x7477:
case 0x7478:
case 0x7479:
case 0x747a:
case 0x747b:
case 0x747c:
case 0x747d:
case 0x747e:
case 0x747f:
case 0x7480:
case 0x7481:
case 0x7482:
case 0x7483:
case 0x7484:
case 0x7485:
case 0x7486:
case 0x7487:
case 0x7488:
case 0x7489:
case 0x748a:
case 0x748b:
case 0x748c:
case 0x748d:
case 0x748e:
case 0x748f:
case 0x7490:
case 0x7491:
case 0x7492:
case 0x7493:
case 0x7494:
case 0x7495:
case 0x7496:
case 0x7497:
case 0x7498:
case 0x7499:
case 0x749a:
case 0x749b:
case 0x749c:
case 0x749d:
case 0x749e:
case 0x749f:
case 0x74a0:
case 0x74a1:
case 0x74a2:
case 0x74a3:
case 0x74a4:
case 0x74a5:
case 0x74a6:
case 0x74a7:
case 0x74a8:
case 0x74a9:
case 0x74aa:
case 0x74ab:
case 0x74ac:
case 0x74ad:
case 0x74ae:
case 0x74af:
case 0x74b0:
case 0x74b1:
case 0x74b2:
case 0x74b3:
case 0x74b4:
case 0x74b5:
case 0x74b6:
case 0x74b7:
case 0x74b8:
case 0x74b9:
case 0x74ba:
case 0x74bb:
case 0x74bc:
case 0x74bd:
case 0x74be:
case 0x74bf:
case 0x74c0:
case 0x74c1:
case 0x74c2:
case 0x74c3:
case 0x74c4:
case 0x74c5:
case 0x74c6:
case 0x74c7:
case 0x74c8:
case 0x74c9:
case 0x74ca:
case 0x74cb:
case 0x74cc:
case 0x74cd:
case 0x74ce:
case 0x74cf:
case 0x74d0:
case 0x74d1:
case 0x74d2:
case 0x74d3:
case 0x74d4:
case 0x74d5:
case 0x74d6:
case 0x74d7:
case 0x74d8:
case 0x74d9:
case 0x74da:
case 0x74db:
case 0x74dc:
case 0x74dd:
case 0x74de:
case 0x74df:
case 0x74e0:
case 0x74e1:
case 0x74e2:
case 0x74e3:
case 0x74e4:
case 0x74e5:
case 0x74e6:
case 0x74e7:
case 0x74e8:
case 0x74e9:
case 0x74ea:
case 0x74eb:
case 0x74ec:
case 0x74ed:
case 0x74ee:
case 0x74ef:
case 0x74f0:
case 0x74f1:
case 0x74f2:
case 0x74f3:
case 0x74f4:
case 0x74f5:
case 0x74f6:
case 0x74f7:
case 0x74f8:
case 0x74f9:
case 0x74fa:
case 0x74fb:
case 0x74fc:
case 0x74fd:
case 0x74fe:
case 0x74ff:
case 0x7500:
case 0x7501:
case 0x7502:
case 0x7503:
case 0x7504:
case 0x7505:
case 0x7506:
case 0x7507:
case 0x7508:
case 0x7509:
case 0x750a:
case 0x750b:
case 0x750c:
case 0x750d:
case 0x750e:
case 0x750f:
case 0x7510:
case 0x7511:
case 0x7512:
case 0x7513:
case 0x7514:
case 0x7515:
case 0x7516:
case 0x7517:
case 0x7518:
case 0x7519:
case 0x751a:
case 0x751b:
case 0x751c:
case 0x751d:
case 0x751e:
case 0x751f:
case 0x7520:
case 0x7521:
case 0x7522:
case 0x7523:
case 0x7524:
case 0x7525:
case 0x7526:
case 0x7527:
case 0x7528:
case 0x7529:
case 0x752a:
case 0x752b:
case 0x752c:
case 0x752d:
case 0x752e:
case 0x752f:
case 0x7530:
case 0x7531:
case 0x7532:
case 0x7533:
case 0x7534:
case 0x7535:
case 0x7536:
case 0x7537:
case 0x7538:
case 0x7539:
case 0x753a:
case 0x753b:
case 0x753c:
case 0x753d:
case 0x753e:
case 0x753f:
case 0x7540:
case 0x7541:
case 0x7542:
case 0x7543:
case 0x7544:
case 0x7545:
case 0x7546:
case 0x7547:
case 0x7548:
case 0x7549:
case 0x754a:
case 0x754b:
case 0x754c:
case 0x754d:
case 0x754e:
case 0x754f:
case 0x7550:
case 0x7551:
case 0x7552:
case 0x7553:
case 0x7554:
case 0x7555:
case 0x7556:
case 0x7557:
case 0x7558:
case 0x7559:
case 0x755a:
case 0x755b:
case 0x755c:
case 0x755d:
case 0x755e:
case 0x755f:
case 0x7560:
case 0x7561:
case 0x7562:
case 0x7563:
case 0x7564:
case 0x7565:
case 0x7566:
case 0x7567:
case 0x7568:
case 0x7569:
case 0x756a:
case 0x756b:
case 0x756c:
case 0x756d:
case 0x756e:
case 0x756f:
case 0x7570:
case 0x7571:
case 0x7572:
case 0x7573:
case 0x7574:
case 0x7575:
case 0x7576:
case 0x7577:
case 0x7578:
case 0x7579:
case 0x757a:
case 0x757b:
case 0x757c:
case 0x757d:
case 0x757e:
case 0x757f:
case 0x7580:
case 0x7581:
case 0x7582:
case 0x7583:
case 0x7584:
case 0x7585:
case 0x7586:
case 0x7587:
case 0x7588:
case 0x7589:
case 0x758a:
case 0x758b:
case 0x758c:
case 0x758d:
case 0x758e:
case 0x758f:
case 0x7590:
case 0x7591:
case 0x7592:
case 0x7593:
case 0x7594:
case 0x7595:
case 0x7596:
case 0x7597:
case 0x7598:
case 0x7599:
case 0x759a:
case 0x759b:
case 0x759c:
case 0x759d:
case 0x759e:
case 0x759f:
case 0x75a0:
case 0x75a1:
case 0x75a2:
case 0x75a3:
case 0x75a4:
case 0x75a5:
case 0x75a6:
case 0x75a7:
case 0x75a8:
case 0x75a9:
case 0x75aa:
case 0x75ab:
case 0x75ac:
case 0x75ad:
case 0x75ae:
case 0x75af:
case 0x75b0:
case 0x75b1:
case 0x75b2:
case 0x75b3:
case 0x75b4:
case 0x75b5:
case 0x75b6:
case 0x75b7:
case 0x75b8:
case 0x75b9:
case 0x75ba:
case 0x75bb:
case 0x75bc:
case 0x75bd:
case 0x75be:
case 0x75bf:
case 0x75c0:
case 0x75c1:
case 0x75c2:
case 0x75c3:
case 0x75c4:
case 0x75c5:
case 0x75c6:
case 0x75c7:
case 0x75c8:
case 0x75c9:
case 0x75ca:
case 0x75cb:
case 0x75cc:
case 0x75cd:
case 0x75ce:
case 0x75cf:
case 0x75d0:
case 0x75d1:
case 0x75d2:
case 0x75d3:
case 0x75d4:
case 0x75d5:
case 0x75d6:
case 0x75d7:
case 0x75d8:
case 0x75d9:
case 0x75da:
case 0x75db:
case 0x75dc:
case 0x75dd:
case 0x75de:
case 0x75df:
case 0x75e0:
case 0x75e1:
case 0x75e2:
case 0x75e3:
case 0x75e4:
case 0x75e5:
case 0x75e6:
case 0x75e7:
case 0x75e8:
case 0x75e9:
case 0x75ea:
case 0x75eb:
case 0x75ec:
case 0x75ed:
case 0x75ee:
case 0x75ef:
case 0x75f0:
case 0x75f1:
case 0x75f2:
case 0x75f3:
case 0x75f4:
case 0x75f5:
case 0x75f6:
case 0x75f7:
case 0x75f8:
case 0x75f9:
case 0x75fa:
case 0x75fb:
case 0x75fc:
case 0x75fd:
case 0x75fe:
case 0x75ff:
case 0x7600:
case 0x7601:
case 0x7602:
case 0x7603:
case 0x7604:
case 0x7605:
case 0x7606:
case 0x7607:
case 0x7608:
case 0x7609:
case 0x760a:
case 0x760b:
case 0x760c:
case 0x760d:
case 0x760e:
case 0x760f:
case 0x7610:
case 0x7611:
case 0x7612:
case 0x7613:
case 0x7614:
case 0x7615:
case 0x7616:
case 0x7617:
case 0x7618:
case 0x7619:
case 0x761a:
case 0x761b:
case 0x761c:
case 0x761d:
case 0x761e:
case 0x761f:
case 0x7620:
case 0x7621:
case 0x7622:
case 0x7623:
case 0x7624:
case 0x7625:
case 0x7626:
case 0x7627:
case 0x7628:
case 0x7629:
case 0x762a:
case 0x762b:
case 0x762c:
case 0x762d:
case 0x762e:
case 0x762f:
case 0x7630:
case 0x7631:
case 0x7632:
case 0x7633:
case 0x7634:
case 0x7635:
case 0x7636:
case 0x7637:
case 0x7638:
case 0x7639:
case 0x763a:
case 0x763b:
case 0x763c:
case 0x763d:
case 0x763e:
case 0x763f:
case 0x7640:
case 0x7641:
case 0x7642:
case 0x7643:
case 0x7644:
case 0x7645:
case 0x7646:
case 0x7647:
case 0x7648:
case 0x7649:
case 0x764a:
case 0x764b:
case 0x764c:
case 0x764d:
case 0x764e:
case 0x764f:
case 0x7650:
case 0x7651:
case 0x7652:
case 0x7653:
case 0x7654:
case 0x7655:
case 0x7656:
case 0x7657:
case 0x7658:
case 0x7659:
case 0x765a:
case 0x765b:
case 0x765c:
case 0x765d:
case 0x765e:
case 0x765f:
case 0x7660:
case 0x7661:
case 0x7662:
case 0x7663:
case 0x7664:
case 0x7665:
case 0x7666:
case 0x7667:
case 0x7668:
case 0x7669:
case 0x766a:
case 0x766b:
case 0x766c:
case 0x766d:
case 0x766e:
case 0x766f:
case 0x7670:
case 0x7671:
case 0x7672:
case 0x7673:
case 0x7674:
case 0x7675:
case 0x7676:
case 0x7677:
case 0x7678:
case 0x7679:
case 0x767a:
case 0x767b:
case 0x767c:
case 0x767d:
case 0x767e:
case 0x767f:
case 0x7680:
case 0x7681:
case 0x7682:
case 0x7683:
case 0x7684:
case 0x7685:
case 0x7686:
case 0x7687:
case 0x7688:
case 0x7689:
case 0x768a:
case 0x768b:
case 0x768c:
case 0x768d:
case 0x768e:
case 0x768f:
case 0x7690:
case 0x7691:
case 0x7692:
case 0x7693:
case 0x7694:
case 0x7695:
case 0x7696:
case 0x7697:
case 0x7698:
case 0x7699:
case 0x769a:
case 0x769b:
case 0x769c:
case 0x769d:
case 0x769e:
case 0x769f:
case 0x76a0:
case 0x76a1:
case 0x76a2:
case 0x76a3:
case 0x76a4:
case 0x76a5:
case 0x76a6:
case 0x76a7:
case 0x76a8:
case 0x76a9:
case 0x76aa:
case 0x76ab:
case 0x76ac:
case 0x76ad:
case 0x76ae:
case 0x76af:
case 0x76b0:
case 0x76b1:
case 0x76b2:
case 0x76b3:
case 0x76b4:
case 0x76b5:
case 0x76b6:
case 0x76b7:
case 0x76b8:
case 0x76b9:
case 0x76ba:
case 0x76bb:
case 0x76bc:
case 0x76bd:
case 0x76be:
case 0x76bf:
case 0x76c0:
case 0x76c1:
case 0x76c2:
case 0x76c3:
case 0x76c4:
case 0x76c5:
case 0x76c6:
case 0x76c7:
case 0x76c8:
case 0x76c9:
case 0x76ca:
case 0x76cb:
case 0x76cc:
case 0x76cd:
case 0x76ce:
case 0x76cf:
case 0x76d0:
case 0x76d1:
case 0x76d2:
case 0x76d3:
case 0x76d4:
case 0x76d5:
case 0x76d6:
case 0x76d7:
case 0x76d8:
case 0x76d9:
case 0x76da:
case 0x76db:
case 0x76dc:
case 0x76dd:
case 0x76de:
case 0x76df:
case 0x76e0:
case 0x76e1:
case 0x76e2:
case 0x76e3:
case 0x76e4:
case 0x76e5:
case 0x76e6:
case 0x76e7:
case 0x76e8:
case 0x76e9:
case 0x76ea:
case 0x76eb:
case 0x76ec:
case 0x76ed:
case 0x76ee:
case 0x76ef:
case 0x76f0:
case 0x76f1:
case 0x76f2:
case 0x76f3:
case 0x76f4:
case 0x76f5:
case 0x76f6:
case 0x76f7:
case 0x76f8:
case 0x76f9:
case 0x76fa:
case 0x76fb:
case 0x76fc:
case 0x76fd:
case 0x76fe:
case 0x76ff:
case 0x7700:
case 0x7701:
case 0x7702:
case 0x7703:
case 0x7704:
case 0x7705:
case 0x7706:
case 0x7707:
case 0x7708:
case 0x7709:
case 0x770a:
case 0x770b:
case 0x770c:
case 0x770d:
case 0x770e:
case 0x770f:
case 0x7710:
case 0x7711:
case 0x7712:
case 0x7713:
case 0x7714:
case 0x7715:
case 0x7716:
case 0x7717:
case 0x7718:
case 0x7719:
case 0x771a:
case 0x771b:
case 0x771c:
case 0x771d:
case 0x771e:
case 0x771f:
case 0x7720:
case 0x7721:
case 0x7722:
case 0x7723:
case 0x7724:
case 0x7725:
case 0x7726:
case 0x7727:
case 0x7728:
case 0x7729:
case 0x772a:
case 0x772b:
case 0x772c:
case 0x772d:
case 0x772e:
case 0x772f:
case 0x7730:
case 0x7731:
case 0x7732:
case 0x7733:
case 0x7734:
case 0x7735:
case 0x7736:
case 0x7737:
case 0x7738:
case 0x7739:
case 0x773a:
case 0x773b:
case 0x773c:
case 0x773d:
case 0x773e:
case 0x773f:
case 0x7740:
case 0x7741:
case 0x7742:
case 0x7743:
case 0x7744:
case 0x7745:
case 0x7746:
case 0x7747:
case 0x7748:
case 0x7749:
case 0x774a:
case 0x774b:
case 0x774c:
case 0x774d:
case 0x774e:
case 0x774f:
case 0x7750:
case 0x7751:
case 0x7752:
case 0x7753:
case 0x7754:
case 0x7755:
case 0x7756:
case 0x7757:
case 0x7758:
case 0x7759:
case 0x775a:
case 0x775b:
case 0x775c:
case 0x775d:
case 0x775e:
case 0x775f:
case 0x7760:
case 0x7761:
case 0x7762:
case 0x7763:
case 0x7764:
case 0x7765:
case 0x7766:
case 0x7767:
case 0x7768:
case 0x7769:
case 0x776a:
case 0x776b:
case 0x776c:
case 0x776d:
case 0x776e:
case 0x776f:
case 0x7770:
case 0x7771:
case 0x7772:
case 0x7773:
case 0x7774:
case 0x7775:
case 0x7776:
case 0x7777:
case 0x7778:
case 0x7779:
case 0x777a:
case 0x777b:
case 0x777c:
case 0x777d:
case 0x777e:
case 0x777f:
case 0x7780:
case 0x7781:
case 0x7782:
case 0x7783:
case 0x7784:
case 0x7785:
case 0x7786:
case 0x7787:
case 0x7788:
case 0x7789:
case 0x778a:
case 0x778b:
case 0x778c:
case 0x778d:
case 0x778e:
case 0x778f:
case 0x7790:
case 0x7791:
case 0x7792:
case 0x7793:
case 0x7794:
case 0x7795:
case 0x7796:
case 0x7797:
case 0x7798:
case 0x7799:
case 0x779a:
case 0x779b:
case 0x779c:
case 0x779d:
case 0x779e:
case 0x779f:
case 0x77a0:
case 0x77a1:
case 0x77a2:
case 0x77a3:
case 0x77a4:
case 0x77a5:
case 0x77a6:
case 0x77a7:
case 0x77a8:
case 0x77a9:
case 0x77aa:
case 0x77ab:
case 0x77ac:
case 0x77ad:
case 0x77ae:
case 0x77af:
case 0x77b0:
case 0x77b1:
case 0x77b2:
case 0x77b3:
case 0x77b4:
case 0x77b5:
case 0x77b6:
case 0x77b7:
case 0x77b8:
case 0x77b9:
case 0x77ba:
case 0x77bb:
case 0x77bc:
case 0x77bd:
case 0x77be:
case 0x77bf:
case 0x77c0:
case 0x77c1:
case 0x77c2:
case 0x77c3:
case 0x77c4:
case 0x77c5:
case 0x77c6:
case 0x77c7:
case 0x77c8:
case 0x77c9:
case 0x77ca:
case 0x77cb:
case 0x77cc:
case 0x77cd:
case 0x77ce:
case 0x77cf:
case 0x77d0:
case 0x77d1:
case 0x77d2:
case 0x77d3:
case 0x77d4:
case 0x77d5:
case 0x77d6:
case 0x77d7:
case 0x77d8:
case 0x77d9:
case 0x77da:
case 0x77db:
case 0x77dc:
case 0x77dd:
case 0x77de:
case 0x77df:
case 0x77e0:
case 0x77e1:
case 0x77e2:
case 0x77e3:
case 0x77e4:
case 0x77e5:
case 0x77e6:
case 0x77e7:
case 0x77e8:
case 0x77e9:
case 0x77ea:
case 0x77eb:
case 0x77ec:
case 0x77ed:
case 0x77ee:
case 0x77ef:
case 0x77f0:
case 0x77f1:
case 0x77f2:
case 0x77f3:
case 0x77f4:
case 0x77f5:
case 0x77f6:
case 0x77f7:
case 0x77f8:
case 0x77f9:
case 0x77fa:
case 0x77fb:
case 0x77fc:
case 0x77fd:
case 0x77fe:
case 0x77ff:
case 0x7800:
case 0x7801:
case 0x7802:
case 0x7803:
case 0x7804:
case 0x7805:
case 0x7806:
case 0x7807:
case 0x7808:
case 0x7809:
case 0x780a:
case 0x780b:
case 0x780c:
case 0x780d:
case 0x780e:
case 0x780f:
case 0x7810:
case 0x7811:
case 0x7812:
case 0x7813:
case 0x7814:
case 0x7815:
case 0x7816:
case 0x7817:
case 0x7818:
case 0x7819:
case 0x781a:
case 0x781b:
case 0x781c:
case 0x781d:
case 0x781e:
case 0x781f:
case 0x7820:
case 0x7821:
case 0x7822:
case 0x7823:
case 0x7824:
case 0x7825:
case 0x7826:
case 0x7827:
case 0x7828:
case 0x7829:
case 0x782a:
case 0x782b:
case 0x782c:
case 0x782d:
case 0x782e:
case 0x782f:
case 0x7830:
case 0x7831:
case 0x7832:
case 0x7833:
case 0x7834:
case 0x7835:
case 0x7836:
case 0x7837:
case 0x7838:
case 0x7839:
case 0x783a:
case 0x783b:
case 0x783c:
case 0x783d:
case 0x783e:
case 0x783f:
case 0x7840:
case 0x7841:
case 0x7842:
case 0x7843:
case 0x7844:
case 0x7845:
case 0x7846:
case 0x7847:
case 0x7848:
case 0x7849:
case 0x784a:
case 0x784b:
case 0x784c:
case 0x784d:
case 0x784e:
case 0x784f:
case 0x7850:
case 0x7851:
case 0x7852:
case 0x7853:
case 0x7854:
case 0x7855:
case 0x7856:
case 0x7857:
case 0x7858:
case 0x7859:
case 0x785a:
case 0x785b:
case 0x785c:
case 0x785d:
case 0x785e:
case 0x785f:
case 0x7860:
case 0x7861:
case 0x7862:
case 0x7863:
case 0x7864:
case 0x7865:
case 0x7866:
case 0x7867:
case 0x7868:
case 0x7869:
case 0x786a:
case 0x786b:
case 0x786c:
case 0x786d:
case 0x786e:
case 0x786f:
case 0x7870:
case 0x7871:
case 0x7872:
case 0x7873:
case 0x7874:
case 0x7875:
case 0x7876:
case 0x7877:
case 0x7878:
case 0x7879:
case 0x787a:
case 0x787b:
case 0x787c:
case 0x787d:
case 0x787e:
case 0x787f:
case 0x7880:
case 0x7881:
case 0x7882:
case 0x7883:
case 0x7884:
case 0x7885:
case 0x7886:
case 0x7887:
case 0x7888:
case 0x7889:
case 0x788a:
case 0x788b:
case 0x788c:
case 0x788d:
case 0x788e:
case 0x788f:
case 0x7890:
case 0x7891:
case 0x7892:
case 0x7893:
case 0x7894:
case 0x7895:
case 0x7896:
case 0x7897:
case 0x7898:
case 0x7899:
case 0x789a:
case 0x789b:
case 0x789c:
case 0x789d:
case 0x789e:
case 0x789f:
case 0x78a0:
case 0x78a1:
case 0x78a2:
case 0x78a3:
case 0x78a4:
case 0x78a5:
case 0x78a6:
case 0x78a7:
case 0x78a8:
case 0x78a9:
case 0x78aa:
case 0x78ab:
case 0x78ac:
case 0x78ad:
case 0x78ae:
case 0x78af:
case 0x78b0:
case 0x78b1:
case 0x78b2:
case 0x78b3:
case 0x78b4:
case 0x78b5:
case 0x78b6:
case 0x78b7:
case 0x78b8:
case 0x78b9:
case 0x78ba:
case 0x78bb:
case 0x78bc:
case 0x78bd:
case 0x78be:
case 0x78bf:
case 0x78c0:
case 0x78c1:
case 0x78c2:
case 0x78c3:
case 0x78c4:
case 0x78c5:
case 0x78c6:
case 0x78c7:
case 0x78c8:
case 0x78c9:
case 0x78ca:
case 0x78cb:
case 0x78cc:
case 0x78cd:
case 0x78ce:
case 0x78cf:
case 0x78d0:
case 0x78d1:
case 0x78d2:
case 0x78d3:
case 0x78d4:
case 0x78d5:
case 0x78d6:
case 0x78d7:
case 0x78d8:
case 0x78d9:
case 0x78da:
case 0x78db:
case 0x78dc:
case 0x78dd:
case 0x78de:
case 0x78df:
case 0x78e0:
case 0x78e1:
case 0x78e2:
case 0x78e3:
case 0x78e4:
case 0x78e5:
case 0x78e6:
case 0x78e7:
case 0x78e8:
case 0x78e9:
case 0x78ea:
case 0x78eb:
case 0x78ec:
case 0x78ed:
case 0x78ee:
case 0x78ef:
case 0x78f0:
case 0x78f1:
case 0x78f2:
case 0x78f3:
case 0x78f4:
case 0x78f5:
case 0x78f6:
case 0x78f7:
case 0x78f8:
case 0x78f9:
case 0x78fa:
case 0x78fb:
case 0x78fc:
case 0x78fd:
case 0x78fe:
case 0x78ff:
case 0x7900:
case 0x7901:
case 0x7902:
case 0x7903:
case 0x7904:
case 0x7905:
case 0x7906:
case 0x7907:
case 0x7908:
case 0x7909:
case 0x790a:
case 0x790b:
case 0x790c:
case 0x790d:
case 0x790e:
case 0x790f:
case 0x7910:
case 0x7911:
case 0x7912:
case 0x7913:
case 0x7914:
case 0x7915:
case 0x7916:
case 0x7917:
case 0x7918:
case 0x7919:
case 0x791a:
case 0x791b:
case 0x791c:
case 0x791d:
case 0x791e:
case 0x791f:
case 0x7920:
case 0x7921:
case 0x7922:
case 0x7923:
case 0x7924:
case 0x7925:
case 0x7926:
case 0x7927:
case 0x7928:
case 0x7929:
case 0x792a:
case 0x792b:
case 0x792c:
case 0x792d:
case 0x792e:
case 0x792f:
case 0x7930:
case 0x7931:
case 0x7932:
case 0x7933:
case 0x7934:
case 0x7935:
case 0x7936:
case 0x7937:
case 0x7938:
case 0x7939:
case 0x793a:
case 0x793b:
case 0x793c:
case 0x793d:
case 0x793e:
case 0x793f:
case 0x7940:
case 0x7941:
case 0x7942:
case 0x7943:
case 0x7944:
case 0x7945:
case 0x7946:
case 0x7947:
case 0x7948:
case 0x7949:
case 0x794a:
case 0x794b:
case 0x794c:
case 0x794d:
case 0x794e:
case 0x794f:
case 0x7950:
case 0x7951:
case 0x7952:
case 0x7953:
case 0x7954:
case 0x7955:
case 0x7956:
case 0x7957:
case 0x7958:
case 0x7959:
case 0x795a:
case 0x795b:
case 0x795c:
case 0x795d:
case 0x795e:
case 0x795f:
case 0x7960:
case 0x7961:
case 0x7962:
case 0x7963:
case 0x7964:
case 0x7965:
case 0x7966:
case 0x7967:
case 0x7968:
case 0x7969:
case 0x796a:
case 0x796b:
case 0x796c:
case 0x796d:
case 0x796e:
case 0x796f:
case 0x7970:
case 0x7971:
case 0x7972:
case 0x7973:
case 0x7974:
case 0x7975:
case 0x7976:
case 0x7977:
case 0x7978:
case 0x7979:
case 0x797a:
case 0x797b:
case 0x797c:
case 0x797d:
case 0x797e:
case 0x797f:
case 0x7980:
case 0x7981:
case 0x7982:
case 0x7983:
case 0x7984:
case 0x7985:
case 0x7986:
case 0x7987:
case 0x7988:
case 0x7989:
case 0x798a:
case 0x798b:
case 0x798c:
case 0x798d:
case 0x798e:
case 0x798f:
case 0x7990:
case 0x7991:
case 0x7992:
case 0x7993:
case 0x7994:
case 0x7995:
case 0x7996:
case 0x7997:
case 0x7998:
case 0x7999:
case 0x799a:
case 0x799b:
case 0x799c:
case 0x799d:
case 0x799e:
case 0x799f:
case 0x79a0:
case 0x79a1:
case 0x79a2:
case 0x79a3:
case 0x79a4:
case 0x79a5:
case 0x79a6:
case 0x79a7:
case 0x79a8:
case 0x79a9:
case 0x79aa:
case 0x79ab:
case 0x79ac:
case 0x79ad:
case 0x79ae:
case 0x79af:
case 0x79b0:
case 0x79b1:
case 0x79b2:
case 0x79b3:
case 0x79b4:
case 0x79b5:
case 0x79b6:
case 0x79b7:
case 0x79b8:
case 0x79b9:
case 0x79ba:
case 0x79bb:
case 0x79bc:
case 0x79bd:
case 0x79be:
case 0x79bf:
case 0x79c0:
case 0x79c1:
case 0x79c2:
case 0x79c3:
case 0x79c4:
case 0x79c5:
case 0x79c6:
case 0x79c7:
case 0x79c8:
case 0x79c9:
case 0x79ca:
case 0x79cb:
case 0x79cc:
case 0x79cd:
case 0x79ce:
case 0x79cf:
case 0x79d0:
case 0x79d1:
case 0x79d2:
case 0x79d3:
case 0x79d4:
case 0x79d5:
case 0x79d6:
case 0x79d7:
case 0x79d8:
case 0x79d9:
case 0x79da:
case 0x79db:
case 0x79dc:
case 0x79dd:
case 0x79de:
case 0x79df:
case 0x79e0:
case 0x79e1:
case 0x79e2:
case 0x79e3:
case 0x79e4:
case 0x79e5:
case 0x79e6:
case 0x79e7:
case 0x79e8:
case 0x79e9:
case 0x79ea:
case 0x79eb:
case 0x79ec:
case 0x79ed:
case 0x79ee:
case 0x79ef:
case 0x79f0:
case 0x79f1:
case 0x79f2:
case 0x79f3:
case 0x79f4:
case 0x79f5:
case 0x79f6:
case 0x79f7:
case 0x79f8:
case 0x79f9:
case 0x79fa:
case 0x79fb:
case 0x79fc:
case 0x79fd:
case 0x79fe:
case 0x79ff:
case 0x7a00:
case 0x7a01:
case 0x7a02:
case 0x7a03:
case 0x7a04:
case 0x7a05:
case 0x7a06:
case 0x7a07:
case 0x7a08:
case 0x7a09:
case 0x7a0a:
case 0x7a0b:
case 0x7a0c:
case 0x7a0d:
case 0x7a0e:
case 0x7a0f:
case 0x7a10:
case 0x7a11:
case 0x7a12:
case 0x7a13:
case 0x7a14:
case 0x7a15:
case 0x7a16:
case 0x7a17:
case 0x7a18:
case 0x7a19:
case 0x7a1a:
case 0x7a1b:
case 0x7a1c:
case 0x7a1d:
case 0x7a1e:
case 0x7a1f:
case 0x7a20:
case 0x7a21:
case 0x7a22:
case 0x7a23:
case 0x7a24:
case 0x7a25:
case 0x7a26:
case 0x7a27:
case 0x7a28:
case 0x7a29:
case 0x7a2a:
case 0x7a2b:
case 0x7a2c:
case 0x7a2d:
case 0x7a2e:
case 0x7a2f:
case 0x7a30:
case 0x7a31:
case 0x7a32:
case 0x7a33:
case 0x7a34:
case 0x7a35:
case 0x7a36:
case 0x7a37:
case 0x7a38:
case 0x7a39:
case 0x7a3a:
case 0x7a3b:
case 0x7a3c:
case 0x7a3d:
case 0x7a3e:
case 0x7a3f:
case 0x7a40:
case 0x7a41:
case 0x7a42:
case 0x7a43:
case 0x7a44:
case 0x7a45:
case 0x7a46:
case 0x7a47:
case 0x7a48:
case 0x7a49:
case 0x7a4a:
case 0x7a4b:
case 0x7a4c:
case 0x7a4d:
case 0x7a4e:
case 0x7a4f:
case 0x7a50:
case 0x7a51:
case 0x7a52:
case 0x7a53:
case 0x7a54:
case 0x7a55:
case 0x7a56:
case 0x7a57:
case 0x7a58:
case 0x7a59:
case 0x7a5a:
case 0x7a5b:
case 0x7a5c:
case 0x7a5d:
case 0x7a5e:
case 0x7a5f:
case 0x7a60:
case 0x7a61:
case 0x7a62:
case 0x7a63:
case 0x7a64:
case 0x7a65:
case 0x7a66:
case 0x7a67:
case 0x7a68:
case 0x7a69:
case 0x7a6a:
case 0x7a6b:
case 0x7a6c:
case 0x7a6d:
case 0x7a6e:
case 0x7a6f:
case 0x7a70:
case 0x7a71:
case 0x7a72:
case 0x7a73:
case 0x7a74:
case 0x7a75:
case 0x7a76:
case 0x7a77:
case 0x7a78:
case 0x7a79:
case 0x7a7a:
case 0x7a7b:
case 0x7a7c:
case 0x7a7d:
case 0x7a7e:
case 0x7a7f:
case 0x7a80:
case 0x7a81:
case 0x7a82:
case 0x7a83:
case 0x7a84:
case 0x7a85:
case 0x7a86:
case 0x7a87:
case 0x7a88:
case 0x7a89:
case 0x7a8a:
case 0x7a8b:
case 0x7a8c:
case 0x7a8d:
case 0x7a8e:
case 0x7a8f:
case 0x7a90:
case 0x7a91:
case 0x7a92:
case 0x7a93:
case 0x7a94:
case 0x7a95:
case 0x7a96:
case 0x7a97:
case 0x7a98:
case 0x7a99:
case 0x7a9a:
case 0x7a9b:
case 0x7a9c:
case 0x7a9d:
case 0x7a9e:
case 0x7a9f:
case 0x7aa0:
case 0x7aa1:
case 0x7aa2:
case 0x7aa3:
case 0x7aa4:
case 0x7aa5:
case 0x7aa6:
case 0x7aa7:
case 0x7aa8:
case 0x7aa9:
case 0x7aaa:
case 0x7aab:
case 0x7aac:
case 0x7aad:
case 0x7aae:
case 0x7aaf:
case 0x7ab0:
case 0x7ab1:
case 0x7ab2:
case 0x7ab3:
case 0x7ab4:
case 0x7ab5:
case 0x7ab6:
case 0x7ab7:
case 0x7ab8:
case 0x7ab9:
case 0x7aba:
case 0x7abb:
case 0x7abc:
case 0x7abd:
case 0x7abe:
case 0x7abf:
case 0x7ac0:
case 0x7ac1:
case 0x7ac2:
case 0x7ac3:
case 0x7ac4:
case 0x7ac5:
case 0x7ac6:
case 0x7ac7:
case 0x7ac8:
case 0x7ac9:
case 0x7aca:
case 0x7acb:
case 0x7acc:
case 0x7acd:
case 0x7ace:
case 0x7acf:
case 0x7ad0:
case 0x7ad1:
case 0x7ad2:
case 0x7ad3:
case 0x7ad4:
case 0x7ad5:
case 0x7ad6:
case 0x7ad7:
case 0x7ad8:
case 0x7ad9:
case 0x7ada:
case 0x7adb:
case 0x7adc:
case 0x7add:
case 0x7ade:
case 0x7adf:
case 0x7ae0:
case 0x7ae1:
case 0x7ae2:
case 0x7ae3:
case 0x7ae4:
case 0x7ae5:
case 0x7ae6:
case 0x7ae7:
case 0x7ae8:
case 0x7ae9:
case 0x7aea:
case 0x7aeb:
case 0x7aec:
case 0x7aed:
case 0x7aee:
case 0x7aef:
case 0x7af0:
case 0x7af1:
case 0x7af2:
case 0x7af3:
case 0x7af4:
case 0x7af5:
case 0x7af6:
case 0x7af7:
case 0x7af8:
case 0x7af9:
case 0x7afa:
case 0x7afb:
case 0x7afc:
case 0x7afd:
case 0x7afe:
case 0x7aff:
case 0x7b00:
case 0x7b01:
case 0x7b02:
case 0x7b03:
case 0x7b04:
case 0x7b05:
case 0x7b06:
case 0x7b07:
case 0x7b08:
case 0x7b09:
case 0x7b0a:
case 0x7b0b:
case 0x7b0c:
case 0x7b0d:
case 0x7b0e:
case 0x7b0f:
case 0x7b10:
case 0x7b11:
case 0x7b12:
case 0x7b13:
case 0x7b14:
case 0x7b15:
case 0x7b16:
case 0x7b17:
case 0x7b18:
case 0x7b19:
case 0x7b1a:
case 0x7b1b:
case 0x7b1c:
case 0x7b1d:
case 0x7b1e:
case 0x7b1f:
case 0x7b20:
case 0x7b21:
case 0x7b22:
case 0x7b23:
case 0x7b24:
case 0x7b25:
case 0x7b26:
case 0x7b27:
case 0x7b28:
case 0x7b29:
case 0x7b2a:
case 0x7b2b:
case 0x7b2c:
case 0x7b2d:
case 0x7b2e:
case 0x7b2f:
case 0x7b30:
case 0x7b31:
case 0x7b32:
case 0x7b33:
case 0x7b34:
case 0x7b35:
case 0x7b36:
case 0x7b37:
case 0x7b38:
case 0x7b39:
case 0x7b3a:
case 0x7b3b:
case 0x7b3c:
case 0x7b3d:
case 0x7b3e:
case 0x7b3f:
case 0x7b40:
case 0x7b41:
case 0x7b42:
case 0x7b43:
case 0x7b44:
case 0x7b45:
case 0x7b46:
case 0x7b47:
case 0x7b48:
case 0x7b49:
case 0x7b4a:
case 0x7b4b:
case 0x7b4c:
case 0x7b4d:
case 0x7b4e:
case 0x7b4f:
case 0x7b50:
case 0x7b51:
case 0x7b52:
case 0x7b53:
case 0x7b54:
case 0x7b55:
case 0x7b56:
case 0x7b57:
case 0x7b58:
case 0x7b59:
case 0x7b5a:
case 0x7b5b:
case 0x7b5c:
case 0x7b5d:
case 0x7b5e:
case 0x7b5f:
case 0x7b60:
case 0x7b61:
case 0x7b62:
case 0x7b63:
case 0x7b64:
case 0x7b65:
case 0x7b66:
case 0x7b67:
case 0x7b68:
case 0x7b69:
case 0x7b6a:
case 0x7b6b:
case 0x7b6c:
case 0x7b6d:
case 0x7b6e:
case 0x7b6f:
case 0x7b70:
case 0x7b71:
case 0x7b72:
case 0x7b73:
case 0x7b74:
case 0x7b75:
case 0x7b76:
case 0x7b77:
case 0x7b78:
case 0x7b79:
case 0x7b7a:
case 0x7b7b:
case 0x7b7c:
case 0x7b7d:
case 0x7b7e:
case 0x7b7f:
case 0x7b80:
case 0x7b81:
case 0x7b82:
case 0x7b83:
case 0x7b84:
case 0x7b85:
case 0x7b86:
case 0x7b87:
case 0x7b88:
case 0x7b89:
case 0x7b8a:
case 0x7b8b:
case 0x7b8c:
case 0x7b8d:
case 0x7b8e:
case 0x7b8f:
case 0x7b90:
case 0x7b91:
case 0x7b92:
case 0x7b93:
case 0x7b94:
case 0x7b95:
case 0x7b96:
case 0x7b97:
case 0x7b98:
case 0x7b99:
case 0x7b9a:
case 0x7b9b:
case 0x7b9c:
case 0x7b9d:
case 0x7b9e:
case 0x7b9f:
case 0x7ba0:
case 0x7ba1:
case 0x7ba2:
case 0x7ba3:
case 0x7ba4:
case 0x7ba5:
case 0x7ba6:
case 0x7ba7:
case 0x7ba8:
case 0x7ba9:
case 0x7baa:
case 0x7bab:
case 0x7bac:
case 0x7bad:
case 0x7bae:
case 0x7baf:
case 0x7bb0:
case 0x7bb1:
case 0x7bb2:
case 0x7bb3:
case 0x7bb4:
case 0x7bb5:
case 0x7bb6:
case 0x7bb7:
case 0x7bb8:
case 0x7bb9:
case 0x7bba:
case 0x7bbb:
case 0x7bbc:
case 0x7bbd:
case 0x7bbe:
case 0x7bbf:
case 0x7bc0:
case 0x7bc1:
case 0x7bc2:
case 0x7bc3:
case 0x7bc4:
case 0x7bc5:
case 0x7bc6:
case 0x7bc7:
case 0x7bc8:
case 0x7bc9:
case 0x7bca:
case 0x7bcb:
case 0x7bcc:
case 0x7bcd:
case 0x7bce:
case 0x7bcf:
case 0x7bd0:
case 0x7bd1:
case 0x7bd2:
case 0x7bd3:
case 0x7bd4:
case 0x7bd5:
case 0x7bd6:
case 0x7bd7:
case 0x7bd8:
case 0x7bd9:
case 0x7bda:
case 0x7bdb:
case 0x7bdc:
case 0x7bdd:
case 0x7bde:
case 0x7bdf:
case 0x7be0:
case 0x7be1:
case 0x7be2:
case 0x7be3:
case 0x7be4:
case 0x7be5:
case 0x7be6:
case 0x7be7:
case 0x7be8:
case 0x7be9:
case 0x7bea:
case 0x7beb:
case 0x7bec:
case 0x7bed:
case 0x7bee:
case 0x7bef:
case 0x7bf0:
case 0x7bf1:
case 0x7bf2:
case 0x7bf3:
case 0x7bf4:
case 0x7bf5:
case 0x7bf6:
case 0x7bf7:
case 0x7bf8:
case 0x7bf9:
case 0x7bfa:
case 0x7bfb:
case 0x7bfc:
case 0x7bfd:
case 0x7bfe:
case 0x7bff:
case 0x7c00:
case 0x7c01:
case 0x7c02:
case 0x7c03:
case 0x7c04:
case 0x7c05:
case 0x7c06:
case 0x7c07:
case 0x7c08:
case 0x7c09:
case 0x7c0a:
case 0x7c0b:
case 0x7c0c:
case 0x7c0d:
case 0x7c0e:
case 0x7c0f:
case 0x7c10:
case 0x7c11:
case 0x7c12:
case 0x7c13:
case 0x7c14:
case 0x7c15:
case 0x7c16:
case 0x7c17:
case 0x7c18:
case 0x7c19:
case 0x7c1a:
case 0x7c1b:
case 0x7c1c:
case 0x7c1d:
case 0x7c1e:
case 0x7c1f:
case 0x7c20:
case 0x7c21:
case 0x7c22:
case 0x7c23:
case 0x7c24:
case 0x7c25:
case 0x7c26:
case 0x7c27:
case 0x7c28:
case 0x7c29:
case 0x7c2a:
case 0x7c2b:
case 0x7c2c:
case 0x7c2d:
case 0x7c2e:
case 0x7c2f:
case 0x7c30:
case 0x7c31:
case 0x7c32:
case 0x7c33:
case 0x7c34:
case 0x7c35:
case 0x7c36:
case 0x7c37:
case 0x7c38:
case 0x7c39:
case 0x7c3a:
case 0x7c3b:
case 0x7c3c:
case 0x7c3d:
case 0x7c3e:
case 0x7c3f:
case 0x7c40:
case 0x7c41:
case 0x7c42:
case 0x7c43:
case 0x7c44:
case 0x7c45:
case 0x7c46:
case 0x7c47:
case 0x7c48:
case 0x7c49:
case 0x7c4a:
case 0x7c4b:
case 0x7c4c:
case 0x7c4d:
case 0x7c4e:
case 0x7c4f:
case 0x7c50:
case 0x7c51:
case 0x7c52:
case 0x7c53:
case 0x7c54:
case 0x7c55:
case 0x7c56:
case 0x7c57:
case 0x7c58:
case 0x7c59:
case 0x7c5a:
case 0x7c5b:
case 0x7c5c:
case 0x7c5d:
case 0x7c5e:
case 0x7c5f:
case 0x7c60:
case 0x7c61:
case 0x7c62:
case 0x7c63:
case 0x7c64:
case 0x7c65:
case 0x7c66:
case 0x7c67:
case 0x7c68:
case 0x7c69:
case 0x7c6a:
case 0x7c6b:
case 0x7c6c:
case 0x7c6d:
case 0x7c6e:
case 0x7c6f:
case 0x7c70:
case 0x7c71:
case 0x7c72:
case 0x7c73:
case 0x7c74:
case 0x7c75:
case 0x7c76:
case 0x7c77:
case 0x7c78:
case 0x7c79:
case 0x7c7a:
case 0x7c7b:
case 0x7c7c:
case 0x7c7d:
case 0x7c7e:
case 0x7c7f:
case 0x7c80:
case 0x7c81:
case 0x7c82:
case 0x7c83:
case 0x7c84:
case 0x7c85:
case 0x7c86:
case 0x7c87:
case 0x7c88:
case 0x7c89:
case 0x7c8a:
case 0x7c8b:
case 0x7c8c:
case 0x7c8d:
case 0x7c8e:
case 0x7c8f:
case 0x7c90:
case 0x7c91:
case 0x7c92:
case 0x7c93:
case 0x7c94:
case 0x7c95:
case 0x7c96:
case 0x7c97:
case 0x7c98:
case 0x7c99:
case 0x7c9a:
case 0x7c9b:
case 0x7c9c:
case 0x7c9d:
case 0x7c9e:
case 0x7c9f:
case 0x7ca0:
case 0x7ca1:
case 0x7ca2:
case 0x7ca3:
case 0x7ca4:
case 0x7ca5:
case 0x7ca6:
case 0x7ca7:
case 0x7ca8:
case 0x7ca9:
case 0x7caa:
case 0x7cab:
case 0x7cac:
case 0x7cad:
case 0x7cae:
case 0x7caf:
case 0x7cb0:
case 0x7cb1:
case 0x7cb2:
case 0x7cb3:
case 0x7cb4:
case 0x7cb5:
case 0x7cb6:
case 0x7cb7:
case 0x7cb8:
case 0x7cb9:
case 0x7cba:
case 0x7cbb:
case 0x7cbc:
case 0x7cbd:
case 0x7cbe:
case 0x7cbf:
case 0x7cc0:
case 0x7cc1:
case 0x7cc2:
case 0x7cc3:
case 0x7cc4:
case 0x7cc5:
case 0x7cc6:
case 0x7cc7:
case 0x7cc8:
case 0x7cc9:
case 0x7cca:
case 0x7ccb:
case 0x7ccc:
case 0x7ccd:
case 0x7cce:
case 0x7ccf:
case 0x7cd0:
case 0x7cd1:
case 0x7cd2:
case 0x7cd3:
case 0x7cd4:
case 0x7cd5:
case 0x7cd6:
case 0x7cd7:
case 0x7cd8:
case 0x7cd9:
case 0x7cda:
case 0x7cdb:
case 0x7cdc:
case 0x7cdd:
case 0x7cde:
case 0x7cdf:
case 0x7ce0:
case 0x7ce1:
case 0x7ce2:
case 0x7ce3:
case 0x7ce4:
case 0x7ce5:
case 0x7ce6:
case 0x7ce7:
case 0x7ce8:
case 0x7ce9:
case 0x7cea:
case 0x7ceb:
case 0x7cec:
case 0x7ced:
case 0x7cee:
case 0x7cef:
case 0x7cf0:
case 0x7cf1:
case 0x7cf2:
case 0x7cf3:
case 0x7cf4:
case 0x7cf5:
case 0x7cf6:
case 0x7cf7:
case 0x7cf8:
case 0x7cf9:
case 0x7cfa:
case 0x7cfb:
case 0x7cfc:
case 0x7cfd:
case 0x7cfe:
case 0x7cff:
case 0x7d00:
case 0x7d01:
case 0x7d02:
case 0x7d03:
case 0x7d04:
case 0x7d05:
case 0x7d06:
case 0x7d07:
case 0x7d08:
case 0x7d09:
case 0x7d0a:
case 0x7d0b:
case 0x7d0c:
case 0x7d0d:
case 0x7d0e:
case 0x7d0f:
case 0x7d10:
case 0x7d11:
case 0x7d12:
case 0x7d13:
case 0x7d14:
case 0x7d15:
case 0x7d16:
case 0x7d17:
case 0x7d18:
case 0x7d19:
case 0x7d1a:
case 0x7d1b:
case 0x7d1c:
case 0x7d1d:
case 0x7d1e:
case 0x7d1f:
case 0x7d20:
case 0x7d21:
case 0x7d22:
case 0x7d23:
case 0x7d24:
case 0x7d25:
case 0x7d26:
case 0x7d27:
case 0x7d28:
case 0x7d29:
case 0x7d2a:
case 0x7d2b:
case 0x7d2c:
case 0x7d2d:
case 0x7d2e:
case 0x7d2f:
case 0x7d30:
case 0x7d31:
case 0x7d32:
case 0x7d33:
case 0x7d34:
case 0x7d35:
case 0x7d36:
case 0x7d37:
case 0x7d38:
case 0x7d39:
case 0x7d3a:
case 0x7d3b:
case 0x7d3c:
case 0x7d3d:
case 0x7d3e:
case 0x7d3f:
case 0x7d40:
case 0x7d41:
case 0x7d42:
case 0x7d43:
case 0x7d44:
case 0x7d45:
case 0x7d46:
case 0x7d47:
case 0x7d48:
case 0x7d49:
case 0x7d4a:
case 0x7d4b:
case 0x7d4c:
case 0x7d4d:
case 0x7d4e:
case 0x7d4f:
case 0x7d50:
case 0x7d51:
case 0x7d52:
case 0x7d53:
case 0x7d54:
case 0x7d55:
case 0x7d56:
case 0x7d57:
case 0x7d58:
case 0x7d59:
case 0x7d5a:
case 0x7d5b:
case 0x7d5c:
case 0x7d5d:
case 0x7d5e:
case 0x7d5f:
case 0x7d60:
case 0x7d61:
case 0x7d62:
case 0x7d63:
case 0x7d64:
case 0x7d65:
case 0x7d66:
case 0x7d67:
case 0x7d68:
case 0x7d69:
case 0x7d6a:
case 0x7d6b:
case 0x7d6c:
case 0x7d6d:
case 0x7d6e:
case 0x7d6f:
case 0x7d70:
case 0x7d71:
case 0x7d72:
case 0x7d73:
case 0x7d74:
case 0x7d75:
case 0x7d76:
case 0x7d77:
case 0x7d78:
case 0x7d79:
case 0x7d7a:
case 0x7d7b:
case 0x7d7c:
case 0x7d7d:
case 0x7d7e:
case 0x7d7f:
case 0x7d80:
case 0x7d81:
case 0x7d82:
case 0x7d83:
case 0x7d84:
case 0x7d85:
case 0x7d86:
case 0x7d87:
case 0x7d88:
case 0x7d89:
case 0x7d8a:
case 0x7d8b:
case 0x7d8c:
case 0x7d8d:
case 0x7d8e:
case 0x7d8f:
case 0x7d90:
case 0x7d91:
case 0x7d92:
case 0x7d93:
case 0x7d94:
case 0x7d95:
case 0x7d96:
case 0x7d97:
case 0x7d98:
case 0x7d99:
case 0x7d9a:
case 0x7d9b:
case 0x7d9c:
case 0x7d9d:
case 0x7d9e:
case 0x7d9f:
case 0x7da0:
case 0x7da1:
case 0x7da2:
case 0x7da3:
case 0x7da4:
case 0x7da5:
case 0x7da6:
case 0x7da7:
case 0x7da8:
case 0x7da9:
case 0x7daa:
case 0x7dab:
case 0x7dac:
case 0x7dad:
case 0x7dae:
case 0x7daf:
case 0x7db0:
case 0x7db1:
case 0x7db2:
case 0x7db3:
case 0x7db4:
case 0x7db5:
case 0x7db6:
case 0x7db7:
case 0x7db8:
case 0x7db9:
case 0x7dba:
case 0x7dbb:
case 0x7dbc:
case 0x7dbd:
case 0x7dbe:
case 0x7dbf:
case 0x7dc0:
case 0x7dc1:
case 0x7dc2:
case 0x7dc3:
case 0x7dc4:
case 0x7dc5:
case 0x7dc6:
case 0x7dc7:
case 0x7dc8:
case 0x7dc9:
case 0x7dca:
case 0x7dcb:
case 0x7dcc:
case 0x7dcd:
case 0x7dce:
case 0x7dcf:
case 0x7dd0:
case 0x7dd1:
case 0x7dd2:
case 0x7dd3:
case 0x7dd4:
case 0x7dd5:
case 0x7dd6:
case 0x7dd7:
case 0x7dd8:
case 0x7dd9:
case 0x7dda:
case 0x7ddb:
case 0x7ddc:
case 0x7ddd:
case 0x7dde:
case 0x7ddf:
case 0x7de0:
case 0x7de1:
case 0x7de2:
case 0x7de3:
case 0x7de4:
case 0x7de5:
case 0x7de6:
case 0x7de7:
case 0x7de8:
case 0x7de9:
case 0x7dea:
case 0x7deb:
case 0x7dec:
case 0x7ded:
case 0x7dee:
case 0x7def:
case 0x7df0:
case 0x7df1:
case 0x7df2:
case 0x7df3:
case 0x7df4:
case 0x7df5:
case 0x7df6:
case 0x7df7:
case 0x7df8:
case 0x7df9:
case 0x7dfa:
case 0x7dfb:
case 0x7dfc:
case 0x7dfd:
case 0x7dfe:
case 0x7dff:
case 0x7e00:
case 0x7e01:
case 0x7e02:
case 0x7e03:
case 0x7e04:
case 0x7e05:
case 0x7e06:
case 0x7e07:
case 0x7e08:
case 0x7e09:
case 0x7e0a:
case 0x7e0b:
case 0x7e0c:
case 0x7e0d:
case 0x7e0e:
case 0x7e0f:
case 0x7e10:
case 0x7e11:
case 0x7e12:
case 0x7e13:
case 0x7e14:
case 0x7e15:
case 0x7e16:
case 0x7e17:
case 0x7e18:
case 0x7e19:
case 0x7e1a:
case 0x7e1b:
case 0x7e1c:
case 0x7e1d:
case 0x7e1e:
case 0x7e1f:
case 0x7e20:
case 0x7e21:
case 0x7e22:
case 0x7e23:
case 0x7e24:
case 0x7e25:
case 0x7e26:
case 0x7e27:
case 0x7e28:
case 0x7e29:
case 0x7e2a:
case 0x7e2b:
case 0x7e2c:
case 0x7e2d:
case 0x7e2e:
case 0x7e2f:
case 0x7e30:
case 0x7e31:
case 0x7e32:
case 0x7e33:
case 0x7e34:
case 0x7e35:
case 0x7e36:
case 0x7e37:
case 0x7e38:
case 0x7e39:
case 0x7e3a:
case 0x7e3b:
case 0x7e3c:
case 0x7e3d:
case 0x7e3e:
case 0x7e3f:
case 0x7e40:
case 0x7e41:
case 0x7e42:
case 0x7e43:
case 0x7e44:
case 0x7e45:
case 0x7e46:
case 0x7e47:
case 0x7e48:
case 0x7e49:
case 0x7e4a:
case 0x7e4b:
case 0x7e4c:
case 0x7e4d:
case 0x7e4e:
case 0x7e4f:
case 0x7e50:
case 0x7e51:
case 0x7e52:
case 0x7e53:
case 0x7e54:
case 0x7e55:
case 0x7e56:
case 0x7e57:
case 0x7e58:
case 0x7e59:
case 0x7e5a:
case 0x7e5b:
case 0x7e5c:
case 0x7e5d:
case 0x7e5e:
case 0x7e5f:
case 0x7e60:
case 0x7e61:
case 0x7e62:
case 0x7e63:
case 0x7e64:
case 0x7e65:
case 0x7e66:
case 0x7e67:
case 0x7e68:
case 0x7e69:
case 0x7e6a:
case 0x7e6b:
case 0x7e6c:
case 0x7e6d:
case 0x7e6e:
case 0x7e6f:
case 0x7e70:
case 0x7e71:
case 0x7e72:
case 0x7e73:
case 0x7e74:
case 0x7e75:
case 0x7e76:
case 0x7e77:
case 0x7e78:
case 0x7e79:
case 0x7e7a:
case 0x7e7b:
case 0x7e7c:
case 0x7e7d:
case 0x7e7e:
case 0x7e7f:
case 0x7e80:
case 0x7e81:
case 0x7e82:
case 0x7e83:
case 0x7e84:
case 0x7e85:
case 0x7e86:
case 0x7e87:
case 0x7e88:
case 0x7e89:
case 0x7e8a:
case 0x7e8b:
case 0x7e8c:
case 0x7e8d:
case 0x7e8e:
case 0x7e8f:
case 0x7e90:
case 0x7e91:
case 0x7e92:
case 0x7e93:
case 0x7e94:
case 0x7e95:
case 0x7e96:
case 0x7e97:
case 0x7e98:
case 0x7e99:
case 0x7e9a:
case 0x7e9b:
case 0x7e9c:
case 0x7e9d:
case 0x7e9e:
case 0x7e9f:
case 0x7ea0:
case 0x7ea1:
case 0x7ea2:
case 0x7ea3:
case 0x7ea4:
case 0x7ea5:
case 0x7ea6:
case 0x7ea7:
case 0x7ea8:
case 0x7ea9:
case 0x7eaa:
case 0x7eab:
case 0x7eac:
case 0x7ead:
case 0x7eae:
case 0x7eaf:
case 0x7eb0:
case 0x7eb1:
case 0x7eb2:
case 0x7eb3:
case 0x7eb4:
case 0x7eb5:
case 0x7eb6:
case 0x7eb7:
case 0x7eb8:
case 0x7eb9:
case 0x7eba:
case 0x7ebb:
case 0x7ebc:
case 0x7ebd:
case 0x7ebe:
case 0x7ebf:
case 0x7ec0:
case 0x7ec1:
case 0x7ec2:
case 0x7ec3:
case 0x7ec4:
case 0x7ec5:
case 0x7ec6:
case 0x7ec7:
case 0x7ec8:
case 0x7ec9:
case 0x7eca:
case 0x7ecb:
case 0x7ecc:
case 0x7ecd:
case 0x7ece:
case 0x7ecf:
case 0x7ed0:
case 0x7ed1:
case 0x7ed2:
case 0x7ed3:
case 0x7ed4:
case 0x7ed5:
case 0x7ed6:
case 0x7ed7:
case 0x7ed8:
case 0x7ed9:
case 0x7eda:
case 0x7edb:
case 0x7edc:
case 0x7edd:
case 0x7ede:
case 0x7edf:
case 0x7ee0:
case 0x7ee1:
case 0x7ee2:
case 0x7ee3:
case 0x7ee4:
case 0x7ee5:
case 0x7ee6:
case 0x7ee7:
case 0x7ee8:
case 0x7ee9:
case 0x7eea:
case 0x7eeb:
case 0x7eec:
case 0x7eed:
case 0x7eee:
case 0x7eef:
case 0x7ef0:
case 0x7ef1:
case 0x7ef2:
case 0x7ef3:
case 0x7ef4:
case 0x7ef5:
case 0x7ef6:
case 0x7ef7:
case 0x7ef8:
case 0x7ef9:
case 0x7efa:
case 0x7efb:
case 0x7efc:
case 0x7efd:
case 0x7efe:
case 0x7eff:
case 0x7f00:
case 0x7f01:
case 0x7f02:
case 0x7f03:
case 0x7f04:
case 0x7f05:
case 0x7f06:
case 0x7f07:
case 0x7f08:
case 0x7f09:
case 0x7f0a:
case 0x7f0b:
case 0x7f0c:
case 0x7f0d:
case 0x7f0e:
case 0x7f0f:
case 0x7f10:
case 0x7f11:
case 0x7f12:
case 0x7f13:
case 0x7f14:
case 0x7f15:
case 0x7f16:
case 0x7f17:
case 0x7f18:
case 0x7f19:
case 0x7f1a:
case 0x7f1b:
case 0x7f1c:
case 0x7f1d:
case 0x7f1e:
case 0x7f1f:
case 0x7f20:
case 0x7f21:
case 0x7f22:
case 0x7f23:
case 0x7f24:
case 0x7f25:
case 0x7f26:
case 0x7f27:
case 0x7f28:
case 0x7f29:
case 0x7f2a:
case 0x7f2b:
case 0x7f2c:
case 0x7f2d:
case 0x7f2e:
case 0x7f2f:
case 0x7f30:
case 0x7f31:
case 0x7f32:
case 0x7f33:
case 0x7f34:
case 0x7f35:
case 0x7f36:
case 0x7f37:
case 0x7f38:
case 0x7f39:
case 0x7f3a:
case 0x7f3b:
case 0x7f3c:
case 0x7f3d:
case 0x7f3e:
case 0x7f3f:
case 0x7f40:
case 0x7f41:
case 0x7f42:
case 0x7f43:
case 0x7f44:
case 0x7f45:
case 0x7f46:
case 0x7f47:
case 0x7f48:
case 0x7f49:
case 0x7f4a:
case 0x7f4b:
case 0x7f4c:
case 0x7f4d:
case 0x7f4e:
case 0x7f4f:
case 0x7f50:
case 0x7f51:
case 0x7f52:
case 0x7f53:
case 0x7f54:
case 0x7f55:
case 0x7f56:
case 0x7f57:
case 0x7f58:
case 0x7f59:
case 0x7f5a:
case 0x7f5b:
case 0x7f5c:
case 0x7f5d:
case 0x7f5e:
case 0x7f5f:
case 0x7f60:
case 0x7f61:
case 0x7f62:
case 0x7f63:
case 0x7f64:
case 0x7f65:
case 0x7f66:
case 0x7f67:
case 0x7f68:
case 0x7f69:
case 0x7f6a:
case 0x7f6b:
case 0x7f6c:
case 0x7f6d:
case 0x7f6e:
case 0x7f6f:
case 0x7f70:
case 0x7f71:
case 0x7f72:
case 0x7f73:
case 0x7f74:
case 0x7f75:
case 0x7f76:
case 0x7f77:
case 0x7f78:
case 0x7f79:
case 0x7f7a:
case 0x7f7b:
case 0x7f7c:
case 0x7f7d:
case 0x7f7e:
case 0x7f7f:
case 0x7f80:
case 0x7f81:
case 0x7f82:
case 0x7f83:
case 0x7f84:
case 0x7f85:
case 0x7f86:
case 0x7f87:
case 0x7f88:
case 0x7f89:
case 0x7f8a:
case 0x7f8b:
case 0x7f8c:
case 0x7f8d:
case 0x7f8e:
case 0x7f8f:
case 0x7f90:
case 0x7f91:
case 0x7f92:
case 0x7f93:
case 0x7f94:
case 0x7f95:
case 0x7f96:
case 0x7f97:
case 0x7f98:
case 0x7f99:
case 0x7f9a:
case 0x7f9b:
case 0x7f9c:
case 0x7f9d:
case 0x7f9e:
case 0x7f9f:
case 0x7fa0:
case 0x7fa1:
case 0x7fa2:
case 0x7fa3:
case 0x7fa4:
case 0x7fa5:
case 0x7fa6:
case 0x7fa7:
case 0x7fa8:
case 0x7fa9:
case 0x7faa:
case 0x7fab:
case 0x7fac:
case 0x7fad:
case 0x7fae:
case 0x7faf:
case 0x7fb0:
case 0x7fb1:
case 0x7fb2:
case 0x7fb3:
case 0x7fb4:
case 0x7fb5:
case 0x7fb6:
case 0x7fb7:
case 0x7fb8:
case 0x7fb9:
case 0x7fba:
case 0x7fbb:
case 0x7fbc:
case 0x7fbd:
case 0x7fbe:
case 0x7fbf:
case 0x7fc0:
case 0x7fc1:
case 0x7fc2:
case 0x7fc3:
case 0x7fc4:
case 0x7fc5:
case 0x7fc6:
case 0x7fc7:
case 0x7fc8:
case 0x7fc9:
case 0x7fca:
case 0x7fcb:
case 0x7fcc:
case 0x7fcd:
case 0x7fce:
case 0x7fcf:
case 0x7fd0:
case 0x7fd1:
case 0x7fd2:
case 0x7fd3:
case 0x7fd4:
case 0x7fd5:
case 0x7fd6:
case 0x7fd7:
case 0x7fd8:
case 0x7fd9:
case 0x7fda:
case 0x7fdb:
case 0x7fdc:
case 0x7fdd:
case 0x7fde:
case 0x7fdf:
case 0x7fe0:
case 0x7fe1:
case 0x7fe2:
case 0x7fe3:
case 0x7fe4:
case 0x7fe5:
case 0x7fe6:
case 0x7fe7:
case 0x7fe8:
case 0x7fe9:
case 0x7fea:
case 0x7feb:
case 0x7fec:
case 0x7fed:
case 0x7fee:
case 0x7fef:
case 0x7ff0:
case 0x7ff1:
case 0x7ff2:
case 0x7ff3:
case 0x7ff4:
case 0x7ff5:
case 0x7ff6:
case 0x7ff7:
case 0x7ff8:
case 0x7ff9:
case 0x7ffa:
case 0x7ffb:
case 0x7ffc:
case 0x7ffd:
case 0x7ffe:
case 0x7fff:
	{
	 LINEF();
	}
	break;

