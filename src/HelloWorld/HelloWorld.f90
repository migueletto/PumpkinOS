module HelloWorld
use iso_c_binding
implicit none

type, bind(C) :: EventType
  integer(c_int16_t) :: eType
end type EventType

integer(c_int16_t), parameter :: menuEvent    = 21
integer(c_int16_t), parameter :: appStopEvent = 22
integer(c_int16_t), parameter :: frmLoadEvent = 23

integer(c_int16_t), parameter :: mainForm = 1000

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

  function SysHandleEvent(event) result(handled) bind(C, name="SysHandleEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
    logical :: handled
  end function SysHandleEvent

  function MenuHandleEvent(menu, event, error) result(handled) bind(C, name="MenuHandleEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(c_ptr) :: menu
    type(EventType) :: event
    logical :: handled
    integer(c_int16_t) :: error
  end function MenuHandleEvent

  subroutine FrmDispatchEvent(event) bind(C, name="FrmDispatchEvent")
    use iso_c_binding
    import :: EventType
    implicit none
    type(EventType) :: event
  end subroutine FrmDispatchEvent

  function FrmGetActiveForm() result(frm) bind(C, name="FrmGetActiveForm")
    use iso_c_binding
    implicit none
    type(c_ptr) :: frm
  end function FrmGetActiveForm

  subroutine FrmDrawForm(frm) bind(C, name="FrmDrawForm")
    use iso_c_binding
    implicit none
    type(c_ptr) :: frm
  end subroutine FrmDrawForm

  function FrmInitForm(formId) result(frm) bind(C, name="FrmInitForm")
    use iso_c_binding
    implicit none
    integer(c_int16_t), value :: formId
    type(c_ptr) :: frm
  end function FrmInitForm

  subroutine FrmSetActiveForm(frm) bind(C, name="FrmSetActiveForm")
    use iso_c_binding
    implicit none
    type(c_ptr) :: frm
  end subroutine FrmSetActiveForm

  subroutine FrmSetEventHandler(frm) bind(C, name="FrmSetEventHandler")
    use iso_c_binding
    implicit none
    type(c_ptr) :: frm
  end subroutine FrmSetEventHandler
end interface

contains

function MainFormHandleEvent(event) result(handled)
  type(EventType) :: event
  logical :: handled
  handled = .false.
end function MainFormHandleEvent

function ApplicationHandleEvent(event) result(handled)
  type(EventType) :: event
  logical :: handled
  type(c_ptr) :: frm

  handled = .false.

  if (event%eType == 23) then
    frm = FrmInitform(mainForm)
    call FrmSetActiveForm(frm)
    call FrmSetEventHandler(frm)
    handled = .true.
  end if
end function ApplicationHandleEvent

subroutine EventLoop()
  type(EventType) :: event
  integer(c_int16_t) :: error
  type(c_ptr), pointer :: menu => null()

  do
    call EvtGetEvent(event, -1)
    if (SysHandleEvent(event)) continue
    if (MenuHandleEvent(menu, event, error)) continue
    if (ApplicationHandleEvent(event)) continue
    call FrmDispatchEvent(event)
    if (event%eType == appStopEvent) exit
  end do
end subroutine EventLoop

function PilotMain(cmd, cmdPBP, launchFlags) result(r) bind(C, name="PilotMain")
  integer(c_int16_t), value :: cmd
  type(c_ptr) :: cmdPBP
  integer(c_int16_t), value :: launchFlags
  integer(c_int32_t) :: r

  if (cmd == 0) then
    call FrmGotoForm(mainForm)
    call EventLoop()
    call FrmCloseAllForms()
  end if

  r = 0
end function PilotMain

end module HelloWorld
