"""Private PE32 fixture code supply, always inside original manifest bookends."""
import hashlib,struct,subprocess
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def original_fixture(root,out):
    def verify(name):
        with (out/name).open('xb') as f:subprocess.run([root/'tools/original-manifest.sh','verify'],cwd=root,stdout=f,stderr=subprocess.STDOUT,check=True)
    verify('sprite-original-before.log')
    try:
        raw=(root/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(raw).hexdigest()!=BUILD:raise ValueError('Unsupported original sprite fixture')
        pe=struct.unpack_from('<I',raw,60)[0];ns=struct.unpack_from('<H',raw,pe+6)[0];optional=struct.unpack_from('<H',raw,pe+20)[0]
        def at(address,size):
            for i in range(ns):
                row=pe+24+optional+i*40;_,rva,n,offset=struct.unpack_from('<4I',raw,row+8);d=address-0x400000-rva
                if 0<=d and d+size<=n:return raw[offset+d:offset+d+size]
            raise ValueError('Unbacked original sprite bytes')
        parts={};fingerprints={}
        for name,start,end,expected in [('flies',0x5492b0,0x549375,'7e95e36a43a94a2e37262ca4906d9525d16d2648ab871bc34d76ea8fd8339a3c'),('cure',0x549730,0x549815,'c37628191cf937c1b6bdef1a2fe90fd6e5540eb185c2909fe8e37e256cdf4a2a')]:
            data=at(start,end-start)
            if hashlib.sha256(data).hexdigest()!=expected or at(end,5)!=b'\x90'*5:raise ValueError('Original sprite method changed')
            parts['sprite_method_'+name]=data;fingerprints[name]=expected
        for name,address,expected in [('prefix',0x5006a2,'8b560c8b4e04528b56088b0152ff500c'),('continuation',0x5006b2,'e9910100000fbf4e120f'),('store',0x500848,'6689461e')]:
            data=at(address,len(bytes.fromhex(expected)))
            if data.hex()!=expected:raise ValueError('Original primary fragment changed')
            parts['sprite_primary_'+name]=data
        if at(0x50084c,1)!=b'\x8b':raise ValueError('Original private fixture exit changed')
        header='/* Generated from checked immutable PE input; no executable file is patched. */\n'
        for name,data in parts.items():header+='static const u8 '+name+'[]={'+','.join(hex(v) for v in data)+'};\n'
        (out/'kind8-sprite-original.inc').write_text(header)
        return dict(original_sha256=BUILD,method_sha256=fingerprints,original_manifest_verified_before_after=True)
    finally:verify('sprite-original-after.log')
