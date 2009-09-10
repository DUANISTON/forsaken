#if 0 
/*
 *  Copyright (C) 1995, 1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File: d3dcalls.c
 *
 *  Calls to Direct3D objects needed for rendering.  Part of D3DApp.
 *
 *  D3DApp is a collection of helper functions for Direct3D applications.
 *  D3DApp consists of the following files:
 *      d3dapp.h    Main D3DApp header to be included by application
 *      d3dappi.h   Internal header
 *      d3dapp.c    D3DApp functions seen by application.
 *      ddcalls.c   All calls to DirectDraw objects except textures
 *      d3dcalls.c  All calls to Direct3D objects except textures
 *      texture.c   Texture loading and managing texture list
 *      misc.c      Miscellaneous calls
 */

#include "typedefs.h"
#include "d3dappi.h"
#include "tload.h"
#include "new3d.h"

extern  BOOL DontColourKey;
extern	BOOL MipMap;
extern	BOOL	Is3Dfx2;
extern	int		TexturePalettized;
extern	int		TextureRedBPP;
extern	int		TextureGreenBPP;
extern	int		TextureBlueBPP;
extern	int		TextureAlphaBPP;
extern	int		TextureIndexBPP;

BOOL	TriLinear;

/***************************************************************************/
/*                            Creation of D3D                              */
/***************************************************************************/

//LPDIRECT3D lpD3D2;
BOOL
D3DAppICreateD3D(void)
{
	/* Set up Direct3D interface object */
	d3dappi.lpD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3dappi.lpD3D)
	{
		OutputDebugString("couldnt create d3d9\n");
		return FALSE;
	}

	return TRUE;

#if 0
//	LastError = d3dappi.lpDD->lpVtbl->QueryInterface(d3dappi.lpDD,
//                                    &IID_IDirect3D2, (LPVOID*)&lpD3D2);
    
	LastError = d3dappi.lpDD->lpVtbl->QueryInterface(d3dappi.lpDD,
                                    &IID_IDirect3D, (LPVOID*)&d3dappi.lpD3D);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Creation of IDirect3D failed.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    return TRUE;
exit_with_error:
    return FALSE;
#endif
}

/***************************************************************************/
/*                           D3D Device Enumeration                        */
/***************************************************************************/
/*
 * enumDeviceFunc
 * Device enumeration callback.  Record information about the D3D device
 * reported by D3D.
 */

#if 0
static HRESULT
WINAPI enumDeviceFunc(LPGUID lpGuid, LPSTR lpDeviceDescription,
                      LPSTR lpDeviceName, LPD3DDEVICEDESC lpHWDesc,
                      LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext)
{
    lpContext = lpContext;
    /*
     * Don't accept any hardware D3D devices if emulation only option is set
     */
    if (lpHWDesc->dcmColorModel && d3dappi.bOnlyEmulation)
        return D3DENUMRET_OK;
    /*
     * Record the D3D driver's inforamation
     */
    memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Guid, lpGuid, sizeof(GUID));
    lstrcpy(d3dappi.Driver[d3dappi.NumDrivers].About, lpDeviceDescription);
    lstrcpy(d3dappi.Driver[d3dappi.NumDrivers].Name, lpDeviceName);
    /*
     * Is this a hardware device or software emulation?  Checking the color
     * model for a valid model works.
     */
    if (lpHWDesc->dcmColorModel) {
        d3dappi.Driver[d3dappi.NumDrivers].bIsHardware = TRUE;
        memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Desc, lpHWDesc,
               sizeof(D3DDEVICEDESC));
    } else {
        d3dappi.Driver[d3dappi.NumDrivers].bIsHardware = FALSE;
        memcpy(&d3dappi.Driver[d3dappi.NumDrivers].Desc, lpHELDesc,
               sizeof(D3DDEVICEDESC));
    }
    /*
     * Does this driver do texture mapping?
     */
    d3dappi.Driver[d3dappi.NumDrivers].bDoesTextures =
        (d3dappi.Driver[d3dappi.NumDrivers].Desc.dpcTriCaps.dwTextureCaps &
         D3DPTEXTURECAPS_PERSPECTIVE) ? TRUE : FALSE;
    /*
     * Does this driver only do Square Textures
     */
    d3dappi.Driver[d3dappi.NumDrivers].bSquareOnly =
        (d3dappi.Driver[d3dappi.NumDrivers].Desc.dpcTriCaps.dwTextureCaps &
         D3DPTEXTURECAPS_SQUAREONLY) ? TRUE : FALSE;

    /*
     * Does this driver do Colour Key Transparency....
     */
    d3dappi.Driver[d3dappi.NumDrivers].bTransparency =
        (d3dappi.Driver[d3dappi.NumDrivers].Desc.dpcTriCaps.dwTextureCaps &
         D3DPTEXTURECAPS_TRANSPARENCY) ? TRUE : FALSE;
    /*
     * Can this driver use a z-buffer?
     */
    d3dappi.Driver[d3dappi.NumDrivers].bDoesZBuffer =
        d3dappi.Driver[d3dappi.NumDrivers].Desc.dwDeviceZBufferBitDepth
                ? TRUE : FALSE;
    /*
     * Can this driver render to the Windows display depth
     */
    d3dappi.Driver[d3dappi.NumDrivers].bCanDoWindow =
        (d3dappi.Driver[d3dappi.NumDrivers].Desc.dwDeviceRenderBitDepth &
         D3DAppIBPPToDDBD(d3dappi.WindowsDisplay.bpp)) ? TRUE : FALSE;
    if (!d3dappi.bIsPrimary)
        d3dappi.Driver[d3dappi.NumDrivers].bCanDoWindow = FALSE;

    d3dappi.NumDrivers++;
    if (d3dappi.NumDrivers == D3DAPP_MAXD3DDRIVERS)
        return (D3DENUMRET_CANCEL);
    return (D3DENUMRET_OK);
}
#endif

/*
 * D3DAppIEnumDrivers
 * Get the available drivers from Direct3D by enumeration.
 */
BOOL
D3DAppIEnumDrivers(void)
{
	return TRUE;
#if 0 // bjd
    d3dappi.NumDrivers = 0;
    LastError = d3dappi.lpD3D->lpVtbl->EnumDevices(d3dappi.lpD3D,
                                                   enumDeviceFunc, NULL);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Enumeration of drivers failed.\n%s",
                              D3DAppErrorToString(LastError));
        return FALSE;
    }
    d3dappi.CurrDriver = 0;
    return TRUE;
#endif
}

#if 0 
static BOOL TextureFormatMatch( D3DAppTextureFormat *t )
{
	return TRUE;
#if 0 // bjd
	if ( t && TexturePalettized >= 0 )
	{
		if ( t->bPalettized && TexturePalettized
			&& t->IndexBPP == TextureIndexBPP ) 
			return TRUE;
		if ( !t->bPalettized && !TexturePalettized
			&& t->RedBPP == TextureRedBPP
			&& t->GreenBPP == TextureGreenBPP
			&& t->BlueBPP == TextureBlueBPP
			&& t->AlphaBPP == TextureAlphaBPP )
			return TRUE;
	}
	return FALSE;
#endif
}
#endif

int	LowestTexFormat;		// used to  select the lowest bit depth for  textures;
/***************************************************************************/
/*                    Enumeration of texure format                         */
/***************************************************************************/
/*
 * EnumTextureFormatsCallback
 * Record information about each texture format the current D3D driver can
 * support. Choose one as the default format (paletted formats are prefered)
 * and return it through lpContext.
 */

#if 0
static HRESULT
CALLBACK EnumTextureFormatsCallback(LPDDSURFACEDESC lpDDSD, LPVOID lpContext)
{
    unsigned long m;
    int r, g, b, a;
    int *lpStartFormat = (int *)lpContext;
	D3DAppTextureFormat *tf_default, *tf_this;


	if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
		return DDENUMRET_OK;

    /*
     * Record the DDSURFACEDESC of this texture format
     */
    memset(&d3dappi.TextureFormat[d3dappi.NumTextureFormats], 0,
           sizeof(D3DAppTextureFormat));
    memcpy(&d3dappi.TextureFormat[d3dappi.NumTextureFormats].ddsd, lpDDSD,
           sizeof(DDSURFACEDESC));
    /*
     * Is this format palettized?  How many bits?  Otherwise, how many RGB
     * bits?
     */
    if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
		// 8 bit palettized is fine if you dont have mipmaps....
		if( MipMap )
			return DDENUMRET_OK;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = TRUE;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 8;
    } else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4) {
		// We dont want 4 bit palettized!!!!!
		return DDENUMRET_OK;
//        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = TRUE;
//        d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 4;
    }else{
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized = FALSE;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP = 0;
        for (r = 0, m = lpDDSD->ddpfPixelFormat.dwRBitMask; !(m & 1);
                                                               r++, m >>= 1);
        for (r = 0; m & 1; r++, m >>= 1);
        for (g = 0, m = lpDDSD->ddpfPixelFormat.dwGBitMask; !(m & 1);
                                                               g++, m >>= 1);
        for (g = 0; m & 1; g++, m >>= 1);
        for (b = 0, m = lpDDSD->ddpfPixelFormat.dwBBitMask; !(m & 1);
                                                               b++, m >>= 1);
        for (b = 0; m & 1; b++, m >>= 1);

		a = 0;
		if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS) {
			for (a = 0, m = lpDDSD->ddpfPixelFormat.dwRGBAlphaBitMask; !(m & 1);
                                                               a++, m >>= 1);
			for (a = 0; m & 1; a++, m >>= 1);
		} 
		// we dont want less than 4 bits per gun...
		if( r+g+b <= 8 )
			return DDENUMRET_OK;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].RedBPP = r;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].GreenBPP = g;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].BlueBPP = b;
        d3dappi.TextureFormat[d3dappi.NumTextureFormats].AlphaBPP = a;
    }
    /*
     * If lpStarFormat is -1, this is the first format.  Select it.
     */
	tf_default = ( *lpStartFormat >= 0 && *lpStartFormat < d3dappi.NumTextureFormats ) ? &d3dappi.TextureFormat[*lpStartFormat] : NULL;
    if (*lpStartFormat == -1)
	{
        *lpStartFormat = d3dappi.NumTextureFormats;
		LowestTexFormat	= r+g+b ;				// makes a note of the bit depths...
		if( LowestTexFormat	< 15 )
			LowestTexFormat = 32;
	}else if ( !TextureFormatMatch( tf_default ) ) {
		tf_this = &d3dappi.TextureFormat[d3dappi.NumTextureFormats];
		/* 
		 * If this format is paletted and at least 8 bit, select it . but not if MipMap is enabled....
		 */
		if ( (	(d3dappi.TextureFormat[d3dappi.NumTextureFormats].bPalettized ) &&
				(d3dappi.TextureFormat[d3dappi.NumTextureFormats].IndexBPP == 8 ) && !MipMap ) )
		{
			if ( TextureFormatMatch( tf_this ) )
			{
				*lpStartFormat = d3dappi.NumTextureFormats; // only select 8-bit palettized if chosen by user
			}
		}else{
			if ( TextureFormatMatch( tf_this ) )
			{
				*lpStartFormat = d3dappi.NumTextureFormats;
				LowestTexFormat	= r+g+b ;	   // makes a note of the lowest bit depth so far...
			}
			else if( d3dappi.Driver[d3dappi.CurrDriver].bTransparency && !DontColourKey )
			{
				// Driver does do Colourkey Transparency...
				if ( ( r+g+b < LowestTexFormat && r+g+b >= 15 && a == 0 ) ) // is this the lowest bit depth????...
				{
					*lpStartFormat = d3dappi.NumTextureFormats;
					LowestTexFormat	= r+g+b ;	   // makes a note of the lowest bit depth so far...
				}
				
			}else{
				// Driver does not do Colour key so we need a texture format with some alpha control...
				// or it messes up when we do colourkey...
				if ( ( r+g+b <= LowestTexFormat && r+g+b >= 12 && a != 0 ) ) // is this the lowest bit depth????...
				{
					*lpStartFormat = d3dappi.NumTextureFormats;
					LowestTexFormat	= r+g+b ;	   // makes a note of the lowest bit depth so far...
				}
			}
		}
	}
    d3dappi.NumTextureFormats++;
    return DDENUMRET_OK;
}
#endif

/*
 * D3DAppIEnumTextureFormats
 * Get a list of available texture map formats from the Direct3D driver by
 * enumeration.  Choose a default format (paletted is prefered).
 */
BOOL
D3DAppIEnumTextureFormats(void)
{
	return TRUE;
#if 0 // bjd
    int StartFormat;
    /*
     * Set the default format to -1 to let the callback know it's being 
     * called for the first time.
     */
    StartFormat = -1;
    d3dappi.NumTextureFormats = 0;
    LastError =
         d3dappi.lpD3DDevice->lpVtbl->EnumTextureFormats(d3dappi.lpD3DDevice,
                                                  EnumTextureFormatsCallback,
                                                  (LPVOID)&StartFormat);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Enumeration of texture formats failed.\n%s",
                              D3DAppErrorToString(LastError));
        return FALSE;
    }
    memcpy(&d3dappi.ThisTextureFormat, &d3dappi.TextureFormat[StartFormat],
           sizeof(D3DAppTextureFormat));
    d3dappi.CurrTextureFormat = StartFormat;
	TexturePalettized = d3dappi.ThisTextureFormat.bPalettized;
	TextureRedBPP = d3dappi.ThisTextureFormat.RedBPP;
	TextureGreenBPP = d3dappi.ThisTextureFormat.GreenBPP;
	TextureBlueBPP = d3dappi.ThisTextureFormat.BlueBPP;
	TextureAlphaBPP = d3dappi.ThisTextureFormat.AlphaBPP;
	TextureIndexBPP = d3dappi.ThisTextureFormat.IndexBPP;
    return TRUE;
#endif
}

/***************************************************************************/
/*                               Device creation                           */
/***************************************************************************/
/*
 * D3DAppICreateDevice
 * Create the D3D device and enumerate the texture formats
 */
BOOL
D3DAppICreateDevice(int driver)
{
	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory (&d3dpp, sizeof(d3dpp));
	d3dpp.hDeviceWindow = d3dappi.;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

	return TRUE;

#if 0
    RELEASE(d3dappi.lpD3DDevice);

    if (d3dappi.Driver[driver].bIsHardware && !d3dappi.bBackBufferInVideo) {
        D3DAppISetErrorString("Could not fit the rendering surfaces in video memory for this hardware device.\n");
        goto exit_with_error;
    }

    d3dappi.CurrDriver = driver;
    memcpy(&d3dappi.ThisDriver, &d3dappi.Driver[driver], sizeof(D3DAppD3DDriver));
    LastError =
           d3dappi.lpBackBuffer->lpVtbl->QueryInterface(d3dappi.lpBackBuffer,
                                                &d3dappi.Driver[driver].Guid,
                                              (LPVOID*)&d3dappi.lpD3DDevice);
    if (LastError != DD_OK) {
        D3DAppISetErrorString("Create D3D device failed.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    d3dappi.CurrDriver = driver;
    d3dappi.NumTextureFormats = 0;
    if (d3dappi.Driver[driver].bDoesTextures) {
        if (!D3DAppIEnumTextureFormats())
            goto exit_with_error;
    }

    return TRUE;
exit_with_error:
    RELEASE(d3dappi.lpD3DDevice);
    return FALSE;
#endif
}

/***************************************************************************/
/*                      Setting the render state                           */
/***************************************************************************/
/*
 * D3DAppISetRenderState
 * Create and execute an execute buffer which will set the render state and
 * light state for the current viewport.
 */
static BOOL SetUpZBuf( DWORD type )
{
	return TRUE;
#if 0 // bjd
	D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPVOID lpBuffer, lpInsStart;
    size_t size;
	LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf;

	lpD3DExCmdBuf = NULL;

    /*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
        return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE);
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
        d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
                                             &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;
    /*
     * Set render state
     */
    OP_STATE_RENDER(1, lpBuffer);

	STATE_DATA(D3DRENDERSTATE_ZFUNC, type, lpBuffer);

    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
                                                          (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);

    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
                                                     lpD3DExCmdBuf,
                                                     d3dappi.lpD3DViewport,
                                                     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }

    RELEASE( lpD3DExCmdBuf );

    return TRUE;

exit_with_error:
    RELEASE( lpD3DExCmdBuf );
    return FALSE;
#endif
}

extern BOOL ZClearsOn;
extern BOOL g_OddFrame;

BOOL SetZCompare( void )
{
	if( !ZClearsOn && g_OddFrame )
	{
		if( !SetUpZBuf( D3DCMP_GREATEREQUAL ) )
			return FALSE;
	}else
	{
		if( !SetUpZBuf( D3DCMP_LESSEQUAL ) )
			return FALSE;
	}

    return TRUE;
}

BOOL
D3DAppISetRenderState()
{
	return TRUE;
#if 0 // bjd
    D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf = NULL;
    LPVOID lpBuffer, lpInsStart;
    size_t size;

    /*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
        return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE) * 20;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
        d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
                                             &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;
    /*
     * Set render state
     */
    OP_STATE_RENDER(17, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_SHADEMODE, d3dapprs.ShadeMode, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_TEXTUREPERSPECTIVE, d3dapprs.bPerspCorrect,
                 lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ZENABLE, d3dapprs.bZBufferOn &&
                                  d3dappi.ThisDriver.bDoesZBuffer, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ZWRITEENABLE, d3dapprs.bZBufferOn, lpBuffer);

      STATE_DATA(D3DRENDERSTATE_ZFUNC, D3DCMP_LESS, lpBuffer);


#ifdef TRILINEAR_MENU_OPTION
	    STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, d3dapprs.TextureFilter,lpBuffer);
	    STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, d3dapprs.TextureFilter,lpBuffer);
#else
	  if( !MipMap )
	  {
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, d3dapprs.TextureFilter,lpBuffer);
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, d3dapprs.TextureFilter,lpBuffer);
	  }else if( !Is3Dfx2 && !TriLinear )
	  {
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_MIPLINEAR,lpBuffer);
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPLINEAR,lpBuffer);
	  }else{
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEARMIPLINEAR,lpBuffer);
		  STATE_DATA(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEARMIPLINEAR,lpBuffer);
	  }
#endif
	  
	  STATE_DATA(D3DRENDERSTATE_TEXTUREMAPBLEND, d3dapprs.TextureBlend,
                 lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FILLMODE, d3dapprs.FillMode, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_DITHERENABLE, d3dapprs.bDithering, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_SPECULARENABLE, d3dapprs.bSpecular,lpBuffer);
      STATE_DATA(D3DRENDERSTATE_ANTIALIAS, d3dapprs.bAntialiasing, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGENABLE, d3dapprs.bFogEnabled, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGCOLOR, d3dapprs.FogColor, lpBuffer);
	  STATE_DATA(D3DRENDERSTATE_FOGTABLEMODE , D3DFOG_LINEAR , lpBuffer);
	  STATE_DATA(D3DRENDERSTATE_FOGTABLESTART , *(unsigned long*)&d3dapprs.FogStart , lpBuffer);
	  STATE_DATA(D3DRENDERSTATE_FOGTABLEEND , *(unsigned long*)&d3dapprs.FogEnd , lpBuffer);

#if 0
	 /*
     * Set light state
     */
    OP_STATE_LIGHT(3, lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGMODE, d3dapprs.bFogEnabled ?
                 d3dapprs.FogMode : D3DFOG_NONE, lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGSTART, *(unsigned long*)&d3dapprs.FogStart,
                 lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGEND, *(unsigned long*)&d3dapprs.FogEnd,
                 lpBuffer);
#endif
    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
                                                          (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);
    LastError = d3dappi.lpD3DDevice->lpVtbl->BeginScene(d3dappi.lpD3DDevice);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("BeginScene failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
                                                     lpD3DExCmdBuf,
                                                     d3dappi.lpD3DViewport,
                                                     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->EndScene(d3dappi.lpD3DDevice);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("EndScene failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * We are done with the execute buffer, so release it.
     */
    lpD3DExCmdBuf->lpVtbl->Release(lpD3DExCmdBuf);
    return TRUE;

exit_with_error:
    RELEASE(lpD3DExCmdBuf);
    return FALSE;
#endif
}


/***************************************************************************/
/*                      Setting the render state                           */
/***************************************************************************/
/*
 * D3DAppISetRenderState
 * Create and execute an execute buffer which will set the render state and
 * light state for the current viewport.
 */
BOOL FogOn( float Start , float End )
{
	return TRUE;
#if 0 // bjd
    D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf = NULL;
    LPVOID lpBuffer, lpInsStart;
    size_t size;
	long lStart;
	long lEnd;

	lStart = (long)Start;
	lEnd = (long)End;
    
	/*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
        return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE) * 20;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
        d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
                                             &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;

    /*
     * Set render state
     */
    OP_STATE_RENDER(2, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGENABLE, 1, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGCOLOR, d3dapprs.FogColor, lpBuffer);

	 /*
     * Set light state
     */
    OP_STATE_LIGHT(3, lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGMODE, D3DFOG_LINEAR , lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGSTART, *(unsigned long*)&lStart,lpBuffer);
      STATE_DATA(D3DLIGHTSTATE_FOGEND, *(unsigned long*)&lEnd,lpBuffer);

    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
                                                          (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);
//    LastError = d3dappi.lpD3DDevice->lpVtbl->BeginScene(d3dappi.lpD3DDevice);
//    if (LastError != D3D_OK) {
//        D3DAppISetErrorString("BeginScene failed in SetRenderState.\n%s",
//                              D3DAppErrorToString(LastError));
//        goto exit_with_error;
//    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
                                                     lpD3DExCmdBuf,
                                                     d3dappi.lpD3DViewport,
                                                     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
//    LastError = d3dappi.lpD3DDevice->lpVtbl->EndScene(d3dappi.lpD3DDevice);
//    if (LastError != D3D_OK) {
//        D3DAppISetErrorString("EndScene failed in SetRenderState.\n%s",
//                              D3DAppErrorToString(LastError));
//        goto exit_with_error;
//    }
    /*
     * We are done with the execute buffer, so release it.
     */
    lpD3DExCmdBuf->lpVtbl->Release(lpD3DExCmdBuf);
    return TRUE;

exit_with_error:
    RELEASE(lpD3DExCmdBuf);
    return FALSE;
#endif
}

/***************************************************************************/
/*                      Setting the render state                           */
/***************************************************************************/
/*
 * D3DAppISetRenderState
 * Create and execute an execute buffer which will set the render state and
 * light state for the current viewport.
 */
BOOL FogOff( void )
{
	return TRUE;
#if 0 //bjd
    D3DEXECUTEBUFFERDESC debDesc;
    D3DEXECUTEDATA d3dExData;
    LPDIRECT3DEXECUTEBUFFER lpD3DExCmdBuf = NULL;
    LPVOID lpBuffer, lpInsStart;
    size_t size;
	/*
     * If there is no D3D Viewport, we must return true because it is not
     * required.
     */
    if (!d3dappi.lpD3DViewport)
        return TRUE;
    /*
     * Create an execute buffer of the required size
     */
    size = 0;
    size += sizeof(D3DINSTRUCTION) * 3;
    size += sizeof(D3DSTATE) * 20;
    memset(&debDesc, 0, sizeof(D3DEXECUTEBUFFERDESC));
    debDesc.dwSize = sizeof(D3DEXECUTEBUFFERDESC);
    debDesc.dwFlags = D3DDEB_BUFSIZE;
    debDesc.dwBufferSize = size;
    LastError =
        d3dappi.lpD3DDevice->lpVtbl->CreateExecuteBuffer(d3dappi.lpD3DDevice,
                                             &debDesc, &lpD3DExCmdBuf, NULL);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("CreateExecuteBuffer failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Lock the execute buffer so it can be filled
     */
    LastError = lpD3DExCmdBuf->lpVtbl->Lock(lpD3DExCmdBuf, &debDesc);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Lock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    memset(debDesc.lpData, 0, size);

    lpInsStart = debDesc.lpData;
    lpBuffer = lpInsStart;
    /*
     * Set render state
     */
    OP_STATE_RENDER(1, lpBuffer);
      STATE_DATA(D3DRENDERSTATE_FOGENABLE, 0, lpBuffer);
    OP_EXIT(lpBuffer);

    LastError = lpD3DExCmdBuf->lpVtbl->Unlock(lpD3DExCmdBuf);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Unlock failed on execute buffer in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
    /*
     * Set the execute data and exectue the buffer
     */
    memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
    d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
    d3dExData.dwInstructionOffset = (ULONG) 0;
    d3dExData.dwInstructionLength = (ULONG) ((char*)lpBuffer -
                                                          (char*)lpInsStart);
    lpD3DExCmdBuf->lpVtbl->SetExecuteData(lpD3DExCmdBuf, &d3dExData);
//    LastError = d3dappi.lpD3DDevice->lpVtbl->BeginScene(d3dappi.lpD3DDevice);
//    if (LastError != D3D_OK) {
//        D3DAppISetErrorString("BeginScene failed in SetRenderState.\n%s",
//                              D3DAppErrorToString(LastError));
//        goto exit_with_error;
//    }
    LastError = d3dappi.lpD3DDevice->lpVtbl->Execute(d3dappi.lpD3DDevice,
                                                     lpD3DExCmdBuf,
                                                     d3dappi.lpD3DViewport,
                                                     D3DEXECUTE_UNCLIPPED);
    if (LastError != D3D_OK) {
        D3DAppISetErrorString("Execute failed in SetRenderState.\n%s",
                              D3DAppErrorToString(LastError));
        goto exit_with_error;
    }
//    LastError = d3dappi.lpD3DDevice->lpVtbl->EndScene(d3dappi.lpD3DDevice);
//    if (LastError != D3D_OK) {
//        D3DAppISetErrorString("EndScene failed in SetRenderState.\n%s",
//                              D3DAppErrorToString(LastError));
//        goto exit_with_error;
//    }
    /*
     * We are done with the execute buffer, so release it.
     */
    lpD3DExCmdBuf->lpVtbl->Release(lpD3DExCmdBuf);
    return TRUE;

exit_with_error:
    RELEASE(lpD3DExCmdBuf);
    return FALSE;
#endif
}

char buf[100];
HRESULT FSGetViewPort(D3DVIEWPORT9 *returnViewPort)
{
	return d3dapp->lpD3DDevice->lpVtbl->GetViewport( d3dapp->lpD3DDevice, returnViewPort );
}

HRESULT FSSetViewPort(D3DVIEWPORT9 *newViewPort)
{
	return d3dapp->lpD3DDevice->lpVtbl->SetViewport( d3dapp->lpD3DDevice, newViewPort );
}
/*
HRESULT FSSetMatrix(D3DMATRIXHANDLE matrixHandle, D3DMATRIX *matrix)
{
	return d3dappi.lpD3DDevice->lpVtbl->SetMatrix(d3dappi.lpD3DDevice, matrixHandle, matrix);
}
*/

HRESULT FSSetMatrix(D3DTRANSFORMSTATETYPE type, const D3DMATRIX *matrix)
{
	return d3dappi.lpD3DDevice->lpVtbl->SetTransform(d3dappi.lpD3DDevice, type, matrix);
}

HRESULT FSGetMatrix(D3DTRANSFORMSTATETYPE type, D3DMATRIX *matrix)
{
	return d3dappi.lpD3DDevice->lpVtbl->GetTransform(d3dappi.lpD3DDevice, type, matrix);
}

HRESULT FSBeginScene()
{
	return d3dappi.lpD3DDevice->lpVtbl->BeginScene(d3dappi.lpD3DDevice);
}

HRESULT FSEndScene()
{
	return d3dappi.lpD3DDevice->lpVtbl->EndScene(d3dappi.lpD3DDevice);
}
/*
HRESULT FSClear(DWORD count, LPD3DRECT rect, DWORD flags)
{
	return d3dappi.lpD3DViewport->lpVtbl->Clear(d3dappi.lpD3DViewport, count, rect, flags);
}
*/
HRESULT FSCreateVertexBuffer(RENDEROBJECT *renderObject, int size)
{
	/*
	HRESULT CreateVertexBuffer(
	  UINT Length,
	  DWORD Usage,
	  DWORD FVF,
	  D3DPOOL Pool,
	  IDirect3DVertexBuffer9** ppVertexBuffer,
	  HANDLE* pSharedHandle
	);
*/
	//LastError = d3d.lpD3DDevice->CreateVertexBuffer(MAX_VERTEXES * sizeof(D3DTLVERTEX),D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_TLVERTEX, D3DPOOL_DEFAULT, &d3d.lpD3DVertexBuffer, NULL);
//	return d3dappi.lpD3DDevice->CreateVertexBuffer(size * sizeof(D3DLVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_LVERTEX, D3DPOOL_DEFAULT, &renderObject->lpD3DVertexBuffer, NULL);
	return d3dappi.lpD3DDevice->lpVtbl->CreateVertexBuffer(d3dappi.lpD3DDevice, (size * sizeof(D3DLVERTEX)), (D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY), D3DFVF_LVERTEX, D3DPOOL_DEFAULT, &renderObject->lpD3DVertexBuffer, NULL);
}

HRESULT FSLockVertexBuffer(RENDEROBJECT *renderObject, LPD3DLVERTEX verts)
{
	//return vertexBufer->Lock(0, 0, verts, D3DLOCK_DISCARD);
	return renderObject->lpD3DVertexBuffer->lpVtbl->Lock(&renderObject->lpD3DVertexBuffer, 0, 0, verts, D3DLOCK_DISCARD);
}

HRESULT FSUnlockVertexBuffer(RENDEROBJECT *renderObject)
{
//	return vertexBufer->Unlock();
	return renderObject->lpD3DVertexBuffer->lpVtbl->Unlock(renderObject->lpD3DVertexBuffer);
}

HRESULT FSDrawVertexBuffer(RENDEROBJECT *renderObject)
{
	HRESULT LastError;

	/* set source */
	LastError = d3dappi.lpD3DDevice->lpVtbl->SetStreamSource(d3dappi.lpD3DDevice, 0, renderObject->lpD3DVertexBuffer, 0, sizeof(D3DLVERTEX));
	if (FAILED(LastError))
	{
		return LastError;
	}

	LastError = d3dappi.lpD3DDevice->lpVtbl->SetFVF(d3dappi.lpD3DDevice, D3DFVF_LVERTEX);
	if (FAILED(LastError))
	{
		return LastError;
	}

	/* set texture */
	LastError = d3dappi.lpD3DDevice->lpVtbl->SetTexture(d3dappi.lpD3DDevice, 0, &renderObject->texture);
	if (FAILED(LastError))
	{
		return LastError;
	}

	/* draw it */
	LastError = d3dappi.lpD3DDevice->lpVtbl->DrawPrimitive(d3dappi.lpD3DDevice, D3DPT_TRIANGLELIST, renderObject->startVert, renderObject->numVerts * 3); // primite count, so multiply by 3
	if (FAILED(LastError))
	{
		return LastError;
	}
	return LastError;
}

void FSReleaseRenderObject(RENDEROBJECT *renderObject)
{
	if (renderObject->lpD3DVertexBuffer)
	{
		renderObject->lpD3DVertexBuffer->lpVtbl->Release(renderObject->lpD3DVertexBuffer);
		renderObject->lpD3DVertexBuffer = NULL;
	}
	
	renderObject->numVerts = 0;
	renderObject->startVert = 0;

	/* don't do this - we need a texture manager and handles to textures */
	renderObject->texture = NULL;
}
#endif