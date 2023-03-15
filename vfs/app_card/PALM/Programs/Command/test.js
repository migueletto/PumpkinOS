// Test

function option1(form) {
  var msg = "";
  if (form.object(form, 2000).value == 1) {
    msg = "A is selected";
  } else if (form.object(form, 2001).value == 1) {
    msg = "B is selected";
  } else {
    msg = "C is selected";
  }
  ui.alert(alert.info, msg);
}

function option2(form) {
  ui.alert(alert.info, string.format("Mark is %d", form.object(form, 3000).value));
}

function buttonClicked(ev, form, obj) {
  ui.alert(alert.info, "clicked " + obj.text);
  return true;
}

mainForm = Form.new({
  title: "Test",
  about: "Test application created with Command on PumpkinOS.",
  menu: [
    Form.menu({ text: "Option 1", handler: option1 }),
    Form.menu({ text: "Option 2", handler: option2 })
  ],
  objects: [
    Form.label({ text: "Just a label" }),
    Form.newline,
    Form.button({ text: "One", handler: buttonClicked }),
    Form.button({ text: "Two", handler: buttonClicked }),
    Form.newline,
    Form.pushButton({ text: "A", group: 1, id: 2000, selected: true }),
    Form.pushButton({ text: "B", group: 1 }),
    Form.pushButton({ text: "C", group: 1 }),
    Form.newline,
    Form.checkbox({ id: 3000, text: "Mark" }),
    Form.newline,
    Form.field({ rows: 2, cols: 10 }),
    Form.list({ visibleItems: 3, items: [ "First", "Second", "Third", "Fourth", "Fifth" ] })
  ]
});

mainForm.show(mainForm);

while (true) {
  ev = ui.event(10);
  if (ev) {
    if (!mainForm.handle(mainForm, ev)) {
      if (ev.type == event.appStop) {
        break;
      }
    }
  }
}
