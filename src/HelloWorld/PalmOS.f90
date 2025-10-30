module PalmOS
use iso_c_binding
implicit none

type, bind(C) :: EventType
  integer(c_int32_t) :: eType
  integer(c_int8_t)  :: penDown
  integer(c_int8_t)  :: tapCount
  integer(c_int16_t) :: screenX
  integer(c_int16_t) :: screenY
  integer(c_int32_t) :: padding
  integer(c_int16_t) :: data(48)
end type EventType

integer(c_int16_t), parameter :: menuEvent    = 21
integer(c_int16_t), parameter :: appStopEvent = 22
integer(c_int16_t), parameter :: frmLoadEvent = 23
integer(c_int16_t), parameter :: frmOpenEvent = 24

integer(c_int16_t), parameter :: sysAppLaunchCmdNormalLaunch = 0

abstract interface
  function EventHandler(event)
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
    logical :: EventHandler
  end function EventHandler
end interface

interface
  subroutine FrmGotoForm(formId) bind(C, name="FrmGotoForm")
    use iso_c_binding
    implicit none
    integer(c_int16_t), value :: formId
  end subroutine FrmGotoForm

  subroutine FrmCloseAllForms() bind(C, name="FrmCloseAllForms")
  end subroutine FrmCloseAllForms

  subroutine EvtGetEvent(event, timeout) bind(C, name="EvtGetEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
    integer(c_int32_t), value :: timeout
  end subroutine EvtGetEvent

  function SysHandleEvent(event) bind(C, name="SysHandleEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
    logical :: SysHandleEvent
  end function SysHandleEvent

  function MenuHandleEvent(menu, event, error) bind(C, name="MenuHandleEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    integer(c_intptr_t), value :: menu
    type(EventType) :: event
    integer(c_int16_t) :: error
    logical :: MenuHandleEvent
  end function MenuHandleEvent

  subroutine FrmDispatchEvent(event) bind(C, name="FrmDispatchEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
  end subroutine FrmDispatchEvent

  function FrmGetActiveForm() bind(C, name="FrmGetActiveForm")
    use iso_c_binding
    implicit none
    type(c_ptr) :: FrmGetActiveForm
  end function FrmGetActiveForm

  subroutine FrmDrawForm(frm) bind(C, name="FrmDrawForm")
    use iso_c_binding
    implicit none
    type(c_ptr), value :: frm
  end subroutine FrmDrawForm

  subroutine FrmDeleteForm(frm) bind(C, name="FrmDeleteForm")
    use iso_c_binding
    implicit none
    type(c_ptr), value :: frm
  end subroutine FrmDeleteForm

  function FrmInitForm(formId) bind(C, name="FrmInitForm")
    use iso_c_binding
    implicit none
    integer(c_int16_t), value :: formId
    type(c_ptr) :: FrmInitForm
  end function FrmInitForm

  function FrmDoDialog(frm) bind(C, name="FrmDoDialog")
    use iso_c_binding
    implicit none
    type(c_ptr), value :: frm
    integer(c_int16_t) :: FrmDoDialog
  end function FrmDoDialog

  subroutine FrmSetActiveForm(frm) bind(C, name="FrmSetActiveForm")
    use iso_c_binding
    implicit none
    type(c_ptr), value :: frm
  end subroutine FrmSetActiveForm

  subroutine FrmSetEventHandler(frm, handler) bind(C, name="FrmSetEventHandler")
    use iso_c_binding
    implicit none
    type(c_ptr), value :: frm
    type(c_ptr), value :: handler
  end subroutine FrmSetEventHandler
end interface

end module PalmOS
