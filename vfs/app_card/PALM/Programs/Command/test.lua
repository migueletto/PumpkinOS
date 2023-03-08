-- Test

mainForm = ui.form(1001, "Test", 0, 0, 160, 160)
ui.button(mainForm, 2001, "Ok", 20, 20)
ui.show(mainForm)

--[[
mainForm = Form.new{
  title = "Test",
  objects = {
    { kind = "button", text = "One", x = 4, y = 20 }
    { kind = "button", text = "Two" }
  }
}
]]--

while true do
  ev = ui.event(10)
  if ev then
    if ev.type == ctlSelect then
      ui.alert(info, "Some message")
    elseif ev.type == appStop then
      break
    end
  end
end
