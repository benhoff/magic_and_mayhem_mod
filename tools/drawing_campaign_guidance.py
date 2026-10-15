"""Bounded original guidance-page attempts; actual Mini state is mandatory."""


def open_mini(physical_action,wait,max_pages=6):
    if not 1<=max_pages<=6:raise ValueError('Guidance page budget')
    attempts=[]
    for page in range(max_pages+1):
        if page:
            physical_action(f'campaign-guidance-page-{page}',dict(kind='click',x=400,y=150,button=1,after_ms=500))
        physical_action(f'campaign-mini-open-{page}',dict(kind='tap',key='Escape',after_ms=500))
        try:
            wait(17,timeout=2)
        except ValueError as error:
            if str(error)!='Original menu state timed out':raise
            attempts.append(dict(page_attempt=page,mini_verified=False))
            continue
        attempts.append(dict(page_attempt=page,mini_verified=True))
        return dict(success=True,attempts=attempts,mini_verified=True,guidance_lifecycle_recovered=False)
    raise ValueError('Guidance page budget exhausted without original Mini')
