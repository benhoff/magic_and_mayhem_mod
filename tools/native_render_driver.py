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


def provision_owned_monitor(output, env):
    """Give a newly owned no-output X server a real RandR monitor rectangle.

    A virtual framebuffer alone can leave Qt with a zero-sized QScreen. This
    affects only the caller's private test server and connects no physical output.
    """
    import subprocess
    from pathlib import Path
    output=Path(output)
    with (output/'hardware-monitor.log').open('x') as log:
        subprocess.run(['xrandr','--setmonitor','NativeRendering',
                        '1280/338x1024/270+0+0','none'],env=env,
                       stdout=log,stderr=subprocess.STDOUT,check=True,timeout=10)
        result=subprocess.run(['xrandr','--listmonitors'],env=env,capture_output=True,
                              text=True,check=True,timeout=10)
        log.write(result.stdout)
    if 'NativeRendering 1280/338x1024/270+0+0' not in result.stdout:
        raise ValueError('Owned private RandR monitor geometry unavailable')
    return dict(width=1280,height=1024,physical_output_connected=False)
