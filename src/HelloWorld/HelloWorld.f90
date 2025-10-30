module HelloWorld
use iso_c_binding
use PalmOS
implicit none

integer(c_int16_t), parameter :: mainForm  = 1000
integer(c_int16_t), parameter :: aboutForm = 1001
integer(c_int16_t), parameter :: aboutCmd  = 1

contains

function MainFormHandleEvent(event)
  type(EventType) :: event
  logical :: MainFormHandleEvent
  type(c_ptr) :: frm
  integer(c_int16_t) :: itemID, buttonID

  if (event%eType == frmOpenEvent) then
    frm = FrmGetActiveForm()
    call FrmDrawForm(frm)
    MainFormHandleEvent = .true.
  else if (event%eType == menuEvent) then
    itemID = event%data(1)
    if (itemID == aboutCmd) then
      frm = FrmInitform(aboutForm)
      buttonID = FrmDoDialog(frm)
      call FrmDeleteForm(frm)
    end if
    MainFormHandleEvent = .true.
  else
    MainFormHandleEvent = .false.
  end if
end function MainFormHandleEvent

function ApplicationHandleEvent(event)
  type(EventType) :: event
  logical :: ApplicationHandleEvent
  type(c_ptr) :: frm
  integer(c_int16_t) :: formID
  procedure(EventHandler), pointer :: eventHandlerPtr

  if (event%eType == frmLoadEvent) then
    formID = event%data(1)
    frm = FrmInitform(formID)
    call FrmSetActiveForm(frm)
    if (formID == mainForm) then
      eventHandlerPtr => MainFormHandleEvent
      call FrmSetEventHandler(frm, eventHandlerPtr)
    end if
    ApplicationHandleEvent = .true.
  else
    ApplicationHandleEvent = .false.
  end if
end function ApplicationHandleEvent

subroutine EventLoop()
  type(EventType) :: event
  integer(c_intptr_t) :: menu = 0
  integer(c_int16_t) :: error

  do
    call EvtGetEvent(event, -1)
    if (SysHandleEvent(event)) cycle
    if (MenuHandleEvent(menu, event, error)) cycle
    if (ApplicationHandleEvent(event)) cycle
    call FrmDispatchEvent(event)
    if (event%eType == appStopEvent) exit
  end do
end subroutine EventLoop

function PilotMain(cmd, cmdPBP, launchFlags) bind(C, name="PilotMain")
  integer(c_int16_t), value :: cmd
  type(c_ptr), value :: cmdPBP
  integer(c_int16_t), value :: launchFlags
  integer(c_int32_t) :: PilotMain

  if (cmd == sysAppLaunchCmdNormalLaunch) then
    call FrmGotoForm(mainForm)
    call EventLoop()
    call FrmCloseAllForms()
  end if

  PilotMain = 0
end function PilotMain

end module HelloWorld
