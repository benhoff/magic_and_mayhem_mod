"""Reviewed stack layouts and bounded buffers, never offset-only COM inference."""

UNKNOWN_WORDS=0xffffffff


def snapshot(argument,size,events,name):
    return {'argument':argument,'bytes':size,'events':events,'name':name}


# PE32 stdcall counts, including the explicit COM This stack argument. Source:
# Wine/Windows wingdi.h, winuser.h, objbase.h, stringapiset.h, amstream.h/mmstream.h
# and the original movie client comparison in tests/drawing-movie-reference.cpp.
IMPORTS={
 ('gdi32.dll','CreatePen'):(3,[]),
 ('gdi32.dll','GetStockObject'):(1,[]),
 ('gdi32.dll','Rectangle'):(5,[]),
 ('gdi32.dll','StretchDIBits'):(13,[snapshot(10,40,1,'BITMAPINFOHEADER prefix')]),
 ('gdi32.dll','GetSystemPaletteEntries'):(4,[]),
 ('gdi32.dll','GetPaletteEntries'):(4,[]),
 ('gdi32.dll','GetDeviceCaps'):(2,[]),
 ('gdi32.dll','GetClipBox'):(2,[snapshot(1,16,2,'RECT output')]),
 ('gdi32.dll','GetDCOrgEx'):(2,[snapshot(1,8,2,'POINT output')]),
 ('user32.dll','GetDC'):(1,[]),
 ('user32.dll','ReleaseDC'):(2,[]),
 ('user32.dll','BeginPaint'):(2,[snapshot(1,64,2,'PAINTSTRUCT output')]),
 ('user32.dll','EndPaint'):(2,[snapshot(1,64,1,'PAINTSTRUCT input')]),
 ('user32.dll','ClientToScreen'):(2,[snapshot(1,8,3,'POINT input/output')]),
 ('user32.dll','DestroyWindow'):(1,[]),
 ('kernel32.dll','GetModuleHandleA'):(1,[]),
 ('kernel32.dll','MultiByteToWideChar'):(6,[]),
 ('ole32.dll','CoCreateInstance'):(5,[snapshot(0,16,1,'CLSID input'),snapshot(3,16,1,'IID input'),snapshot(4,4,2,'interface pointer output')]),
}

# Exact original sites already exercised by the1760-case movie client fixture.
# An arbitrary indirect CALL at the same vtable offset acquires no such type.
MOVIE={
 0x46973b:('ff5130','IAMMultiMediaStream.Initialize',4,[]),
 0x469768:('ff523c','IAMMultiMediaStream.AddMediaStream',5,[snapshot(2,16,1,'purpose GUID input'),snapshot(4,4,2,'media stream pointer output')]),
 0x469793:('ff513c','IAMMultiMediaStream.AddMediaStream',5,[snapshot(2,16,1,'purpose GUID input'),snapshot(4,4,2,'media stream pointer output')]),
 0x4697d1:('ff5140','IAMMultiMediaStream.OpenFile',3,[]),
 0x4697f4:('ff5104','IUnknown.AddRef',1,[]),
 0x469817:('ff5208','IUnknown.Release',1,[]),
 0x469863:('ff5010','IMultiMediaStream.GetMediaStream',3,[snapshot(1,16,1,'purpose GUID input'),snapshot(2,4,2,'media stream pointer output')]),
 0x469890:('ff12','IUnknown.QueryInterface',3,[snapshot(1,16,1,'IID input'),snapshot(2,4,2,'interface pointer output')]),
 0x4698ba:('ff5234','IDirectDrawMediaStream.CreateSample',5,[snapshot(2,16,1,'RECT input'),snapshot(4,4,2,'sample pointer output')]),
 0x4698e7:('ff5220','IDirectDrawStreamSample.GetSurface',3,[snapshot(1,4,2,'surface pointer output'),snapshot(2,16,2,'RECT output')]),
 0x469908:('ff521c','IMultiMediaStream.SetState',2,[]),
 0x469964:('ffd7','original cached GetAsyncKeyState',1,[]),
 0x46997a:('ff5218','IStreamSample.Update',5,[]),
 0x4699a3:('ff5014','IDirectDrawSurface.Blt',6,[snapshot(1,16,1,'destination RECT input'),snapshot(3,16,1,'source RECT input')]),
 0x4699bc:('ff5208','IUnknown.Release',1,[]),
 0x4699ca:('ff5108','IUnknown.Release',1,[]),
 0x4699d8:('ff5208','IUnknown.Release',1,[]),
 0x4699e6:('ff5108','IUnknown.Release',1,[]),
 0x469a48:('ff5108','IUnknown.Release',1,[]),
}


def contract(item,revision=1):
    if type(revision) is not int or revision not in (1,2):raise ValueError('Unsupported exact contract revision')
    if revision==2:
        from drawing_api_surface_contracts import surface_contract
        result=surface_contract(item)
        if result is not None:
            validate(result)
            return result
    imported=item.get('exact_import')
    key=(imported['dll'].lower(),imported['name']) if imported else None
    if key in IMPORTS:
        words,slots=IMPORTS[key];name=imported['dll']+'.'+imported['name'];origin='exact_import'
    elif item['site'] in MOVIE:
        signature,name,words,slots=MOVIE[item['site']]
        if item['call']['bytes']!=signature:raise ValueError('Reviewed movie CALL signature mismatch')
        origin='exact_original_movie_site'
    else:return {'name':'unreviewed','formal_words':UNKNOWN_WORDS,'snapshots':[],'origin':'unreviewed'}
    result={'name':name,'formal_words':words,'snapshots':[dict(slot) for slot in slots],'origin':origin}
    validate(result)
    return result


def validate(value):
    if not isinstance(value,dict) or set(value)!={'name','formal_words','snapshots','origin'} or not isinstance(value['name'],str) or not value['name'] or value['origin'] not in ('exact_import','exact_original_movie_site','exact_original_graphics_site','unreviewed','synthetic_fixture'):
        raise ValueError('Invalid reviewed contract identity')
    words=value['formal_words'];slots=value['snapshots']
    if type(words) is not int or words!=UNKNOWN_WORDS and not 0<=words<=16 or not isinstance(slots,list) or len(slots)>4:
        raise ValueError('Invalid reviewed argument/snapshot bounds')
    if words==UNKNOWN_WORDS and slots:raise ValueError('Unreviewed CALL cannot acquire buffer snapshots')
    for slot in slots:
        if not isinstance(slot,dict) or set(slot)!={'argument','bytes','events','name'} or not isinstance(slot['name'],str) or not slot['name']:
            raise ValueError('Invalid reviewed snapshot identity')
        if any(type(slot[k]) is not int for k in ('argument','bytes','events')) or not 0<=slot['argument']<words or not 1<=slot['bytes']<=128 or slot['events'] not in (1,2,3):
            raise ValueError('Invalid reviewed snapshot pointer/extent/events')
    if value['name'].endswith('.Release') and slots:raise ValueError('Release cannot dereference released objects')
