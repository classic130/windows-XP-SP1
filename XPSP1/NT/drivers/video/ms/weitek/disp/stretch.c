/******************************Module*Header*******************************\
* Module Name: stretch.c
*
* Copyright (c) 1993-1995 Microsoft Corporation
\**************************************************************************/

#include "precomp.h"

#define STRETCH_MAX_EXTENT 32767

typedef DWORDLONG ULONGLONG;

/******************************Public*Routine******************************\
*
* Routine Name
*
*   vDirectStretch8Narrow
*
* Routine Description:
*
*   Stretch blt 8->8 when the width is 7 or less
*
* Arguments:
*
*   pStrBlt - contains all params for blt
*
* Return Value:
*
*   VOID
*
\**************************************************************************/

VOID vDirectStretch8Narrow(
STR_BLT* pStrBlt)
{
    BYTE*   pjSrc;
    BYTE*   pjDstEnd;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;

    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc;
    BYTE*   pjDst       = pStrBlt->pjDstScan + xDst;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - WidthX;
    ULONG   yInt        = 0;

    yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;

    //
    // Narrow blt
    //

    do {

        ULONG  yTmp = yAccum + yFrac;
        BYTE   jSrc0;
        BYTE*  pjDstEndNarrow = pjDst + WidthX;

        pjSrc   = pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        do {
            jSrc0    = *pjSrc;
            xTmp     = xAccum + xFrac;
            pjSrc    = pjSrc + xInt + (xTmp < xAccum);
            *pjDst++ = jSrc0;
            xAccum   = xTmp;
        } while (pjDst != pjDstEndNarrow);

        pjSrcScan += yInt;

        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }

        yAccum = yTmp;
        pjDst += lDstStride;

    } while (--yCount);

}

/******************************Public*Routine******************************\
*
* Routine Name
*
*   vDirectStretch32
*
* Routine Description:
*
*   Stretch blt 32->32
*
* Arguments:
*
*   pStrBlt - contains all params for blt
*
* Return Value:
*
*   VOID
*
\**************************************************************************/

VOID vDirectStretch32(
STR_BLT* pStrBlt)
{
    BYTE*   pjOldScan;
    ULONG*  pulSrc;
    ULONG*  pulDstEnd;
    ULONG   ulDst;
    ULONG   xAccum;
    ULONG   xTmp;
    ULONG   yTmp;
    LONG    cyDuplicate;
    BYTE*   pjBase;

    PDEV*   ppdev       = pStrBlt->ppdev;
    LONG    xDst        = pStrBlt->XDstStart;
    LONG    xSrc        = pStrBlt->XSrcStart;
    BYTE*   pjSrcScan   = pStrBlt->pjSrcScan + xSrc * 4;
    ULONG*  pulDst      = (ULONG*)(pStrBlt->pjDstScan) + xDst;
    LONG    yDst        = pStrBlt->YDstStart;
    LONG    yCount      = pStrBlt->YDstCount;
    LONG    WidthX      = pStrBlt->XDstEnd - xDst;
    ULONG   xInt        = pStrBlt->ulXDstToSrcIntCeil;
    ULONG   xFrac       = pStrBlt->ulXDstToSrcFracCeil;
    ULONG   yAccum      = pStrBlt->ulYFracAccumulator;
    ULONG   yFrac       = pStrBlt->ulYDstToSrcFracCeil;
    LONG    lDstStride  = pStrBlt->lDeltaDst - 4*WidthX;
    ULONG   yInt        = 0;

    pjBase = ppdev->pjBase;

    //
    // if this is a shrinking blt, calc src scan line stride
    //

    if (pStrBlt->ulYDstToSrcIntCeil != 0)
    {
        yInt = pStrBlt->lDeltaSrc * pStrBlt->ulYDstToSrcIntCeil;
    }

    do {

        ULONG   ulSrc;
        ULONG   yTmp;

        pulSrc  = (ULONG*) pjSrcScan;
        xAccum  = pStrBlt->ulXFracAccumulator;

        //
        // a single src scan line is being written
        //

        pulDstEnd  = pulDst + WidthX;

        while (pulDst != pulDstEnd)
        {

            ulSrc  = *pulSrc;
            xTmp   = xAccum + xFrac;
            pulSrc = pulSrc + xInt + (xTmp < xAccum);
            *(ULONG*)pulDst = ulSrc;
            pulDst++;
            xAccum = xTmp;
        }

        pjOldScan = pjSrcScan;
        pjSrcScan += yInt;

        yTmp = yAccum + yFrac;
        if (yTmp < yAccum)
        {
            pjSrcScan += pStrBlt->lDeltaSrc;
        }
        yAccum = yTmp;

        pulDst = (ULONG*) ((BYTE*) pulDst + lDstStride);
        yDst++;
        yCount--;

        if ((yCount != 0) && (pjSrcScan == pjOldScan))
        {
            // It's an expanding stretch in 'y'; the scan we just laid down
            // will be copied at least once using the hardware:

            cyDuplicate = 0;
            do {
                cyDuplicate++;
                pjSrcScan += yInt;

                yTmp = yAccum + yFrac;
                if (yTmp < yAccum)
                {
                    pjSrcScan += pStrBlt->lDeltaSrc;
                }
                yAccum = yTmp;

                pulDst = (ULONG*) ((BYTE*) pulDst + pStrBlt->lDeltaDst);
                yCount--;

            } while ((yCount != 0) && (pjSrcScan == pjOldScan));

            // The scan is to be copied 'cyDuplicate' times using the
            // hardware.

            do {
                CP_Y0(ppdev, pjBase, yDst - 1);
                CP_Y1(ppdev, pjBase, yDst - 1);
                CP_Y2(ppdev, pjBase, yDst);
                CP_Y3(ppdev, pjBase, yDst);
                yDst++;
                CP_START_BLT_WAIT(ppdev, pjBase);
            } while (--cyDuplicate != 0);
        }
    } while (yCount != 0);
}

/******************************Public*Routine******************************\
*
* Routine Description:
*
*   StretchBlt using integer math. Must be from one surface to another
*   surface of the same format.
*
* Arguments:
*
*   ppdev           -   PDEV for device
*   pvDst           -   Pointer to start of dst bitmap
*   lDeltaDst       -   Bytes from start of dst scan line to start of next
*   DstCx           -   Width of Dst Bitmap in pixels
*   DstCy           -   Height of Dst Bitmap in pixels
*   prclDst         -   Pointer to rectangle of Dst extents
*   pvSrc           -   Pointer to start of Src bitmap
*   lDeltaSrc       -   Bytes from start of Src scan line to start of next
*   SrcCx           -   Width of Src Bitmap in pixels
*   SrcCy           -   Height of Src Bitmap in pixels
*   prclSrc         -   Pointer to rectangle of Src extents
*   prclSClip       -   Clip Dest to this rect
*
* Return Value:
*
*   Status
*
\**************************************************************************/

BOOL bStretchDIBDirect(
PDEV*   ppdev,
VOID*   pvDst,
LONG    lDeltaDst,
RECTL*  prclDst,
VOID*   pvSrc,
LONG    lDeltaSrc,
RECTL*  prclSrc,
RECTL*  prclClip)
{
    STR_BLT StrBlt;
    ULONG   XSrcToDstIntFloor;
    ULONG   XSrcToDstFracFloor;
    ULONG   ulXDstToSrcIntCeil;
    ULONG   ulXDstToSrcFracCeil;
    ULONG   YSrcToDstIntFloor;
    ULONG   YSrcToDstFracFloor;
    ULONG   ulYDstToSrcIntCeil;
    ULONG   ulYDstToSrcFracCeil;
    LONG    SrcIntScan;
    LONG    DstDeltaScanEnd;
    ULONG   ulXFracAccumulator;
    ULONG   ulYFracAccumulator;
    LONG    LeftClipDistance;
    LONG    TopClipDistance;
    BOOL    bStretch;

    union {
        LARGE_INTEGER   large;
        ULONGLONG       li;
    } liInit;

    PFN_DIRSTRETCH      pfnStr;

    //
    // Calculate exclusive start and end points:
    //

    LONG    WidthDst  = prclDst->right  - prclDst->left;
    LONG    HeightDst = prclDst->bottom - prclDst->top;
    LONG    WidthSrc  = prclSrc->right  - prclSrc->left;
    LONG    HeightSrc = prclSrc->bottom - prclSrc->top;

    LONG    XSrcStart = prclSrc->left;
    LONG    XSrcEnd   = prclSrc->right;
    LONG    XDstStart = prclDst->left;
    LONG    XDstEnd   = prclDst->right;
    LONG    YSrcStart = prclSrc->top;
    LONG    YSrcEnd   = prclSrc->bottom;
    LONG    YDstStart = prclDst->top;
    LONG    YDstEnd   = prclDst->bottom;

    //
    // Validate parameters:
    //

    ASSERTDD(pvDst != (VOID*)NULL, "Bad destination bitmap pointer");
    ASSERTDD(pvSrc != (VOID*)NULL, "Bad source bitmap pointer");
    ASSERTDD(prclDst != (RECTL*)NULL, "Bad destination rectangle");
    ASSERTDD(prclSrc != (RECTL*)NULL, "Bad source rectangle");
    ASSERTDD((WidthDst > 0) && (HeightDst > 0) &&
             (WidthSrc > 0) && (HeightSrc > 0),
             "Can't do mirroring or empty rectangles here");
    ASSERTDD((WidthDst  <= STRETCH_MAX_EXTENT) &&
             (HeightDst <= STRETCH_MAX_EXTENT) &&
             (WidthSrc  <= STRETCH_MAX_EXTENT) &&
             (HeightSrc <= STRETCH_MAX_EXTENT), "Stretch exceeds limits");
    ASSERTDD(prclClip != NULL, "Bad clip rectangle");

    //
    // Calculate X Dst to Src mapping
    //
    //
    // dst->src =  ( CEIL( (2k*WidthSrc)/WidthDst) ) / 2k
    //
    //          =  ( FLOOR(  (2k*WidthSrc -1) / WidthDst) + 1) / 2k
    //
    // where 2k = 2 ^ 32
    //

    {
        ULONGLONG   liWidthSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

        //
        // Work around a compiler bug dealing with the assignment
        // 'liHeightSrc = (((LONGLONG)HeightSrc) << 32) - 1':
        //

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = WidthSrc - 1;
        liWidthSrc = liInit.li;

        liQuo = liWidthSrc / (ULONGLONG) WidthDst;

        ulXDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulXDstToSrcFracCeil = (ULONG)liQuo;

        //
        // Now add 1, use fake carry:
        //

        ulTemp = ulXDstToSrcFracCeil + 1;

        ulXDstToSrcIntCeil += (ulTemp < ulXDstToSrcFracCeil);
        ulXDstToSrcFracCeil = ulTemp;
    }

    //
    // Calculate Y Dst to Src mapping
    //
    //
    // dst->src =  ( CEIL( (2k*HeightSrc)/HeightDst) ) / 2k
    //
    //          =  ( FLOOR(  (2k*HeightSrc -1) / HeightDst) + 1) / 2k
    //
    // where 2k = 2 ^ 32
    //

    {
        ULONGLONG   liHeightSrc;
        ULONGLONG   liQuo;
        ULONG       ulTemp;

        //
        // Work around a compiler bug dealing with the assignment
        // 'liHeightSrc = (((LONGLONG)HeightSrc) << 32) - 1':
        //

        liInit.large.LowPart = (ULONG) -1;
        liInit.large.HighPart = HeightSrc - 1;
        liHeightSrc = liInit.li;

        liQuo = liHeightSrc / (ULONGLONG) HeightDst;

        ulYDstToSrcIntCeil  = (ULONG)(liQuo >> 32);
        ulYDstToSrcFracCeil = (ULONG)liQuo;

        //
        // Now add 1, use fake carry:
        //

        ulTemp = ulYDstToSrcFracCeil + 1;

        ulYDstToSrcIntCeil += (ulTemp < ulYDstToSrcFracCeil);
        ulYDstToSrcFracCeil = ulTemp;
    }

    //
    // Now clip Dst in X, and/or calc src clipping effect on dst
    //
    // adjust left and right edges if needed, record
    // distance adjusted for fixing the src
    //

    if (XDstStart < prclClip->left)
    {
        XDstStart = prclClip->left;
    }

    if (XDstEnd > prclClip->right)
    {
        XDstEnd = prclClip->right;
    }

    //
    // Check for totally clipped out destination:
    //

    if (XDstEnd <= XDstStart)
    {
        return(TRUE);
    }

    LeftClipDistance = XDstStart - prclDst->left;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

        //
        // Calculate displacement for .5 in destination and add:
        //

        ulTempFrac = (ulXDstToSrcFracCeil >> 1) | (ulXDstToSrcIntCeil << 31);
        ulTempInt  = (ulXDstToSrcIntCeil >> 1);

        XSrcStart += ulTempInt;
        ulXFracAccumulator = ulTempFrac;

        if (LeftClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulXDstToSrcFracCeil, LeftClipDistance);

            ulTmp = ulXFracAccumulator;
            ulXFracAccumulator += (ULONG) (ullFraction);
            if (ulXFracAccumulator < ulTmp)
                XSrcStart++;

            XSrcStart += (ulXDstToSrcIntCeil * LeftClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

    //
    // Now clip Dst in Y, and/or calc src clipping effect on dst
    //
    // adjust top and bottom edges if needed, record
    // distance adjusted for fixing the src
    //

    if (YDstStart < prclClip->top)
    {
        YDstStart = prclClip->top;
    }

    if (YDstEnd > prclClip->bottom)
    {
        YDstEnd = prclClip->bottom;
    }

    //
    // Check for totally clipped out destination:
    //

    if (YDstEnd <= YDstStart)
    {
        return(TRUE);
    }

    TopClipDistance = YDstStart - prclDst->top;

    {
        ULONG   ulTempInt;
        ULONG   ulTempFrac;

        //
        // Calculate displacement for .5 in destination and add:
        //

        ulTempFrac = (ulYDstToSrcFracCeil >> 1) | (ulYDstToSrcIntCeil << 31);
        ulTempInt  = ulYDstToSrcIntCeil >> 1;

        YSrcStart += (LONG)ulTempInt;
        ulYFracAccumulator = ulTempFrac;

        if (TopClipDistance != 0)
        {
            ULONGLONG ullFraction;
            ULONG     ulTmp;

            ullFraction = UInt32x32To64(ulYDstToSrcFracCeil, TopClipDistance);

            ulTmp = ulYFracAccumulator;
            ulYFracAccumulator += (ULONG) (ullFraction);
            if (ulYFracAccumulator < ulTmp)
                YSrcStart++;

            YSrcStart += (ulYDstToSrcIntCeil * TopClipDistance)
                       + (ULONG) (ullFraction >> 32);
        }
    }

    //
    // Warm up the hardware if doing an expanding stretch in 'y':
    //

    bStretch = (HeightDst > HeightSrc);
    if (bStretch)
    {
        LONG    cjPel = ppdev->cjPel;
        BYTE*   pjBase = ppdev->pjBase;
        LONG    x;

        CP_WAIT(ppdev, pjBase);
        CP_RASTER(ppdev, pjBase, P9000(ppdev) ? P9000_S : P9100_S);

        //
        // Set up the left and right blt edges, since they never change
        // during the StretchBlt
        //

        x = (XDstStart + ppdev->xOffset) * cjPel;
        CP_ABS_X0(ppdev, pjBase, x);
        CP_ABS_X2(ppdev, pjBase, x);

        x = (XDstEnd + ppdev->xOffset) * cjPel - 1;
        CP_ABS_X1(ppdev, pjBase, x);
        CP_ABS_X3(ppdev, pjBase, x);
    }

    //
    // Fill out blt structure, then call format-specific stretch code
    //

    StrBlt.ppdev     = ppdev;
    StrBlt.XDstEnd   = XDstEnd;
    StrBlt.YDstStart = YDstStart;
    StrBlt.YDstCount = YDstEnd - YDstStart;

    if (StrBlt.YDstCount > 0)
    {
        //
        // Caclulate starting scan line address.  Since the inner loop
        // routines are format dependent, they must add XDstStart/XSrcStart
        // to pjDstScan/pjSrcScan to get the actual starting pixel address.
        //

        StrBlt.pjSrcScan           = (BYTE*) pvSrc + (YSrcStart * lDeltaSrc);
        StrBlt.pjDstScan           = (BYTE*) pvDst + (YDstStart * lDeltaDst);

        StrBlt.lDeltaSrc           = lDeltaSrc;
        StrBlt.XSrcStart           = XSrcStart;
        StrBlt.XDstStart           = XDstStart;
        StrBlt.lDeltaDst           = lDeltaDst;
        StrBlt.ulXDstToSrcIntCeil  = ulXDstToSrcIntCeil;
        StrBlt.ulXDstToSrcFracCeil = ulXDstToSrcFracCeil;
        StrBlt.ulYDstToSrcIntCeil  = ulYDstToSrcIntCeil;
        StrBlt.ulYDstToSrcFracCeil = ulYDstToSrcFracCeil;
        StrBlt.ulXFracAccumulator  = ulXFracAccumulator;
        StrBlt.ulYFracAccumulator  = ulYFracAccumulator;

        if (ppdev->iBitmapFormat == BMF_8BPP)
        {
            if ((XDstEnd - XDstStart) < 7)
                pfnStr = vDirectStretch8Narrow;
            else
                pfnStr = vDirectStretch8;
        }
        else if (ppdev->iBitmapFormat == BMF_16BPP)
        {
            pfnStr = vDirectStretch16;
        }
        else
        {
            ASSERTDD(ppdev->iBitmapFormat == BMF_32BPP,
                     "Only handle stretchs at 8, 16 and 32bpp");

            pfnStr = vDirectStretch32;
        }

        (*pfnStr)(&StrBlt);
    }

    return(TRUE);
}

/******************************Public*Routine******************************\
* BOOL DrvStretchBlt
*
\**************************************************************************/

BOOL DrvStretchBlt(
SURFOBJ*            psoDst,
SURFOBJ*            psoSrc,
SURFOBJ*            psoMsk,
CLIPOBJ*            pco,
XLATEOBJ*           pxlo,
COLORADJUSTMENT*    pca,
POINTL*             pptlHTOrg,
RECTL*              prclDst,
RECTL*              prclSrc,
POINTL*             pptlMsk,
ULONG               iMode)
{
    DSURF*  pdsurfSrc;
    DSURF*  pdsurfDst;
    PDEV*   ppdev;
    OH*     poh;

    // GDI guarantees us that for a StretchBlt the destination surface
    // will always be a device surface, and not a DIB:

    ppdev = (PDEV*) psoDst->dhpdev;

    // It's quicker for GDI to do a StretchBlt when the source surface
    // is not a device-managed surface, because then it can directly
    // read the source bits without having to allocate a temporary
    // buffer and call DrvCopyBits to get a copy that it can use.
    //
    // So if the source is one of our off-screen DFBs, we'll immediately
    // and permanently convert it to a DIB:

    if (psoSrc->iType == STYPE_DEVBITMAP)
    {
        pdsurfSrc = (DSURF*) psoSrc->dhsurf;
        if (pdsurfSrc->dt == DT_SCREEN)
        {
            if (!pohMoveOffscreenDfbToDib(ppdev, pdsurfSrc->poh))
                return(FALSE);
        }

        ASSERTDD(pdsurfSrc->dt == DT_DIB, "Can only handle DIB DFBs here");

        psoSrc = pdsurfSrc->pso;
    }

    pdsurfDst = (DSURF*) psoDst->dhsurf;
    if (pdsurfDst->dt == DT_DIB)
    {
        // The destination was a device bitmap that we just converted
        // to a DIB:

        psoDst = pdsurfDst->pso;
        goto Punt_It;
    }

    poh             = pdsurfDst->poh;
    psoDst          = ppdev->psoPunt;
    ppdev->xOffset  = poh->x;
    ppdev->yOffset  = poh->y;
    psoDst->pvScan0 = poh->pvScan0;

    // NOTE: The P9000 is allowed down here, too

    {
        RECTL       rclClip;
        RECTL*      prclClip;
        ULONG       cxDst;
        ULONG       cyDst;
        ULONG       cxSrc;
        ULONG       cySrc;
        BOOL        bMore;
        CLIPENUM    ce;
        LONG        c;
        LONG        i;

        if ((psoSrc->iType == STYPE_BITMAP) &&
            (psoMsk == NULL) &&
            ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL)) &&
            ((psoSrc->iBitmapFormat == ppdev->iBitmapFormat)) &&
            (ppdev->iBitmapFormat != BMF_24BPP))
        {
            cxDst = prclDst->right - prclDst->left;
            cyDst = prclDst->bottom - prclDst->top;
            cxSrc = prclSrc->right - prclSrc->left;
            cySrc = prclSrc->bottom - prclSrc->top;

            // Our 'bStretchDIBDirect' routine requires that the stretch be
            // non-inverting, within a certain size, to have no source
            // clipping, and to have no empty rectangles (the latter is the
            // reason for the '- 1' on the unsigned compare here):

            if (((cxSrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cySrc - 1) < STRETCH_MAX_EXTENT)         &&
                ((cxDst - 1) < STRETCH_MAX_EXTENT)         &&
                ((cyDst - 1) < STRETCH_MAX_EXTENT)         &&
                (prclSrc->left   >= 0)                     &&
                (prclSrc->top    >= 0)                     &&
                (prclSrc->right  <= psoSrc->sizlBitmap.cx) &&
                (prclSrc->bottom <= psoSrc->sizlBitmap.cy))
            {
                // Our snazzy routine only does COLORONCOLOR.  But for
                // stretching blts, BLACKONWHITE and WHITEONBLACK are also
                // equivalent to COLORONCOLOR:

                if ((iMode == COLORONCOLOR) ||
                    ((iMode < COLORONCOLOR) && (cxSrc <= cxDst) && (cySrc <= cyDst)))
                {
                    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
                    {
                        rclClip.left   = LONG_MIN;
                        rclClip.top    = LONG_MIN;
                        rclClip.right  = LONG_MAX;
                        rclClip.bottom = LONG_MAX;
                        prclClip = &rclClip;

                    StretchSingleClipRect:

                        if (bStretchDIBDirect(ppdev,
                                        poh->pvScan0,
                                        ppdev->lDelta,
                                        prclDst,
                                        psoSrc->pvScan0,
                                        psoSrc->lDelta,
                                        prclSrc,
                                        prclClip))
                        {
                            return(TRUE);
                        }
                    }
                    else if (pco->iDComplexity == DC_RECT)
                    {
                        prclClip = &pco->rclBounds;
                        goto StretchSingleClipRect;
                    }
                    else
                    {
                        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, CD_ANY, 0);

                        do {
                            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

                            c = cIntersect(prclDst, ce.arcl, ce.c);

                            if (c != 0)
                            {
                                for (i = 0; i < c; i++)
                                {
                                    if (!bStretchDIBDirect(ppdev,
                                                     poh->pvScan0,
                                                     ppdev->lDelta,
                                                     prclDst,
                                                     psoSrc->pvScan0,
                                                     psoSrc->lDelta,
                                                     prclSrc,
                                                     &ce.arcl[i]))
                                    {
                                        goto Punt_It;
                                    }
                                }
                            }

                        } while (bMore);

                        return(TRUE);
                    }
                }
            }
        }
    }

Punt_It:

    return(EngStretchBlt(psoDst, psoSrc, psoMsk, pco, pxlo, pca, pptlHTOrg,
                         prclDst, prclSrc, pptlMsk, iMode));
}
