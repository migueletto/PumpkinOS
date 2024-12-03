#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#define navBitUp          0x0001  // Key mask for the five way navigation UP button
#define navBitDown        0x0002  // Key mask for the five way navigation DOWN button
#define navBitLeft        0x0004  // Key mask for the five way navigation LEFT button
#define navBitRight       0x0008  // Key mask for the five way navigation RIGHT button
#define navBitSelect      0x0010  // Key mask for the five way navigation SELECT button
#define navBitsAll        0x001F  // Key mask for all five way navigation buttons

#define navChangeUp       0x0100  // Key mask for the five way navigation UP button state change
#define navChangeDown     0x0200  // Key mask for the five way navigation DOWN button state change
#define navChangeLeft     0x0400  // Key mask for the five way navigation LEFT button state change
#define navChangeRight    0x0800  // Key mask for the five way navigation RIGHT button state change
#define navChangeSelect   0x1000  // Key mask for the five way navigation SELECT button state change
#define navChangeBitsAll  0x1F00  // Key mask for all five way navigation buttons state change

#define vchrNavChange  (vchrPalmMin + 3)

#define TxtCharIsRockerKey(m, c) ((((m) & commandKeyMask) != 0) && ((((c) >= vchrRockerUp) && ((c) <= vchrRockerCenter))))

#define IsFiveWayNavPalmEvent(eventP)                                                       \
(                                                                                           \
    (   ((eventP)->data.keyDown.chr == vchrPageUp)                                          \
     || ((eventP)->data.keyDown.chr == vchrPageDown)                                        \
     || ((eventP)->data.keyDown.chr == vchrNavChange) )                                     \
&&  (((eventP)->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) != 0)               \
)

#define IsFiveWayNavEvent(eventP) \
(                                                                                           \
    IsFiveWayNavPalmEvent(eventP)                                                           \
||  TxtCharIsRockerKey((eventP)->data.keyDown.modifiers, (eventP)->data.keyDown.chr)        \
)

#define NavSelectHSPressed(eventP) \
( \
       (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0)                       \
    && ((eventP)->data.keyDown.chr == vchrRockerCenter)                                 \
)

#define NavSelectPalmPressed(eventP) \
( \
       (((eventP)->data.keyDown.modifiers & autoRepeatKeyMask) == 0) \
    && ((eventP)->data.keyDown.chr == vchrNavChange)                                    \
    && (((eventP)->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) == navChangeSelect)   \
)

#define NavSelectPressed(eventP) \
( \
 IsFiveWayNavEvent(eventP) \
&&  (   NavSelectPalmPressed(eventP)                                                    \
     || NavSelectHSPressed(eventP))                                                     \
)

#define NavDirectionHSPressed(eventP, nav)                                              \
(                                                                                       \
        ((vchrRocker ## nav) != vchrRockerCenter)                                       \
     && (   (   (((eventP)->data.keyDown.modifiers & commandKeyMask) != 0)              \
             && ((eventP)->data.keyDown.chr == (vchrRocker ## nav))))                   \
)

#define NavDirectionPalmPressed(eventP, nav)                                            \
(                                                                                       \
    ((eventP)->data.keyDown.modifiers & autoRepeatKeyMask)                              \
        ? (((eventP)->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) ==        \
            (navBit ## nav))                                                            \
        : (((eventP)->data.keyDown.keyCode & (navBitsAll | navChangeBitsAll)) ==        \
            (navBit ## nav | navChange ## nav))                                         \
)

#define NavDirectionPressed(eventP, nav)                                                \
(                                                                                       \
    IsFiveWayNavEvent(eventP)                                                           \
        ? (   NavDirectionPalmPressed(eventP, nav)                                      \
           || NavDirectionHSPressed(eventP, nav))                                       \
        : (   ((eventP)->data.keyDown.chr == vchrPageUp && (navBit ## nav) == navBitUp)      \
      || ((eventP)->data.keyDown.chr == vchrPageDown && (navBit ## nav) == navBitDown)) \
)

#define NavKeyPressed(eventP, nav)                                                      \
(                                                                                       \
    (navBit ## nav == navBitSelect)                                                     \
        ? NavSelectPressed(eventP)                                                      \
        : NavDirectionPressed(eventP, nav)                                              \
)

#endif
