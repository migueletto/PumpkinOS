-- Test

function option1(form)
  local msg = ""
  if form:object(2000):value() == 1 then
    msg = "A is selected"
  elseif form:object(2001):value() == 1 then
    msg = "B is selected"
  else
    msg = "C is selected"
  end
  ui.alert(alert.info, msg)
end

function option2(form)
  local obj = form:object(3000)
  ui.alert(alert.info, string.format("Mark is %d", obj:value()))
end

function buttonClicked(ev)
  ui.alert(alert.info, "clicked " .. ev.label)
  return true
end

mainForm = Form.new {
  title = "Test",
  about = "Test application created with Command on PumpkinOS.",
  menu = {
    Form.menu { text = "Option 1", handler = option1 },
    Form.menu { text = "Option 2", handler = option2 }
  },
  objects = {
    Form.label { text = "Just a label" },
    Form.newline,
    Form.button { text = "One", handler = buttonClicked },
    Form.button { text = "Two", handler = buttonClicked },
    Form.newline,
    Form.pushButton { text = "A", group = 1, id = 2000, selected = true },
    Form.pushButton { text = "B", group = 1 },
    Form.pushButton { text = "C", group = 1 },
    Form.newline,
    Form.checkbox { id = 3000, text = "Mark" },
    Form.newline,
    Form.field { rows = 2, cols = 10 }
  }
}

mainForm:show()

while true do
  ev = ui.event(10)
  if ev then
    if not mainForm:handle(ev) then
      if ev.type == event.appStop then
        break
      end
    end
  end
end
