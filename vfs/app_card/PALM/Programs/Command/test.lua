-- Test

function option1()
  ui.alert(alert.info, "Menu option 1")
end

function option2()
  ui.alert(alert.info, "Menu option 2")
end

function buttonClicked(ev)
  ui.alert(alert.info, "clicked " .. ev.label)
  return true
end

mainForm = Form.new{
  title = "Test",
  about = "Test application created with Command on PumpkinOS.",
  menu = {
    Form.menu{ text = "Option 1", handler = option1 },
    Form.menu{ text = "Option 2", handler = option2 }
  },
  objects = {
    Form.label{ text = "Just a label" },
    Form.newline,
    Form.button{ text = "One", handler = buttonClicked },
    Form.button{ text = "Two", handler = buttonClicked },
    Form.newline,
    Form.pushButton{ text = "A", group = 1, selected = true },
    Form.pushButton{ text = "B", group = 1 },
    Form.pushButton{ text = "C", group = 1 },
    Form.newline,
    Form.field{ rows = 2, cols = 10 }
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
