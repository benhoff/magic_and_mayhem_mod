"""Fail closed on missing or software contexts in the declared NVIDIA profile."""


def require_nvidia(drivers):
    if not isinstance(drivers,list) or not drivers:
        raise ValueError('Missing observed NVIDIA renderer identities')
    for driver in drivers:
        if (not isinstance(driver,str) or 'NVIDIA' not in driver or
                any(name in driver.casefold() for name in ('llvmpipe','lavapipe','softpipe','swrast','software','swiftshader','osmesa'))):
            raise ValueError('Actual renderer did not retain NVIDIA hardware identity: '+str(driver))
    return dict(verified=True,observed_drivers=drivers,software_fallback=False,
                scope='Observed GL renderer identities for the declared NVIDIA profile; not scanout or sustained performance proof')
