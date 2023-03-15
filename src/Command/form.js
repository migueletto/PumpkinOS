// Form class

Form = {};

Form.newline = "newline";

Form.menu = function(def) {
  if (!def.cmd)  def.cmd = 0;
  if (!def.text) def.text = "";
  return def;
}

Form.label = function(def) {
  def.kind = "label";
  if (!def.text) def.text = "";
  if (!def.font) def.font = font.bold;
  return def;
}

Form.button = function(def) {
  def.kind = "button";
  if (!def.text) def.text = "";
  if (!def.font) def.font = font.std;
  if (!def.repeating) def.repeating = false; else def.repeating = true;
  return def;
}

Form.pushButton = function(def) {
  def.kind = "pushButton";
  if (!def.group) def.group = 1;
  if (!def.text)  def.text = "";
  if (!def.font)  def.font = font.std;
  return def;
}

Form.checkbox = function(def) {
  def.kind = "checkbox";
  if (!def.text) def.text = "";
  if (!def.font) def.font = font.std;
  return def;
}

Form.slider = function(def) {
  def.kind = "slider";
  if (!def.text)  def.text = "";
  if (!def.font)  def.font = font.std;
  if (!def.min)   def.min  = 1;
  if (!def.max)   def.max  = 10;
  if (!def.page)  def.page = 1;
  if (!def.feedback) def.feedback = false; else def.feedback = true;
  return def;
}

Form.selector = function(def) {
  def.kind = "selector";
  if (!def.text) def.text = "";
  if (!def.font) def.font = font.std;
  return def;
}

Form.field = function(def) {
  def.kind = "field"
  if (!def.cols) def.cols = 16;
  if (!def.rows) def.rows = 1;
  if (!def.max)  def.max = def.rows * def.cols;
  if (!def.font) def.font = font.std;
  return def;
}

Form.list = function(def) {
  def.kind = "list";
  if (!def.width) def.width = 0;
  if (!def.height) def.height = 0;
  if (!def.visibleItems) def.visibleItems = 0;
  if (!def.items) def.items = ""; 
  if (!def.font) def.font = font.std;
  return def;
}

Form.popup = function(def) {
  def.kind = "popup";
  if (!def.visibleItems) def.visibleItems = 0;
  if (!def.font) def.font = font.std;
  return def;
}

Form.new = function(f) {
  if (!f.id) f.id = 1000;
  if (!f.title) f.title = "";
  if (!f.x) f.x = 0;
  if (!f.y) f.y = 0;
  if (!f.width) f.width = 160;
  if (!f.height) f.height = 160;

  f.first = true;
  f.ptr = ui.form(f.id, f.title, f.x, f.y, f.width, f.height);

  f.menuHandlers = {};
  f.formHandlers = {};
  f.objectsById  = {};

  if (!f.about) {
    f.about = "Application created with Command.";
  }
  f.menuHandlers[1] = function() {
    ui.about(f.about);
  }

  if (f.objects) {
    var id = 1000;
    var x = 4;
    var y = 20;
    var h = 0;
    for (key in f.objects) {
      var obj = f.objects[key];
      if (typeof obj == "string") {
        if (obj == Form.newline) {
          x = 4;
          y = y + h + 6;
          h = 0;
        }
      } else if (typeof obj == "object") {
        if (!obj.x) obj.x = x;
        if (!obj.y) obj.y = y;
        if (!obj.id) {
          obj.id = id;
          id = id + 1;
        } else {
          id = obj.id + 1;
        }

        obj.form = f;

        if (obj.kind == "label") {
          ui.label(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 6;
          if (obj.height > h) {
            h = obj.height;
          }
        } else if (obj.kind == "button") {
          if (obj.repeating) {
            ui.rbutton(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font);
          } else {
            ui.button(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font);
          }
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 6;
          if (obj.height > h) {
            h = obj.height;
          }
          if (obj.handler) {
            f.formHandlers[obj.id] = obj.handler;
          }
        } else if (obj.kind == "pushButton") {
          ui.pushbutton(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, obj.group, obj.selected);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 1;
          if (obj.height > h) {
            h = obj.height;
          }
          if (obj.handler) {
            f.formHandlers[obj.id] = obj.handler;
          }
        } else if (obj.kind == "checkbox") {
          ui.checkbox(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, obj.selected);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 6;
          if (obj.height > h) {
            h = obj.height;
          }
          if (obj.handler) {
            f.formHandlers[obj.id] = obj.handler;
          }
        } else if (obj.kind == "slider") {
          if (obj.feedback) {
            ui.fslider(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, false, obj.min, obj.max, obj.page);
          } else {
            ui.slider(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, false, obj.min, obj.max, obj.page);
          }
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 6;
          if (obj.height > h) {
            h = obj.height;
          }
          if (obj.handler) {
            f.formHandlers[obj.id] = obj.handler;
          }
        } else if (obj.kind == "selector") {
          ui.selector(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 1;
          if (obj.height > h) {
            h = obj.height;
          }
          if (obj.handler) {
            f.formHandlers[obj.id] = obj.handler
          }
        } else if (obj.kind == "field") {
          ui.field(f.ptr, obj.id, obj.x, obj.y, obj.cols, obj.rows, obj.max, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 4
          if (obj.height > h) {
            h = obj.height
          }
        } else if (obj.kind == "list") {
          ui.list(f.ptr, obj.id, obj.x, obj.y, obj.width, obj.height, obj.visibleItems, obj.font, obj.items);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 4;
          if (obj.height > h) {
            h = obj.height;
          }
        } else if (obj.kind == "popup") {
          ui.popup(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.visibleItems, obj.font, obj.items);
          ui.bounds(f.ptr, obj.id, obj);
          x = x + obj.width + 4;
          if (obj.height > h) {
            h = obj.height;
          }
        }

        f.objectsById[obj.id] = obj;

        obj.x = null;
        obj.y = null;
        obj.width = null;
        obj.height = null;
        obj.value = null;
        obj.text = null;

        Object.defineProperty(obj, "value", {
          get: function() {
            return ui.getvalue(this.form.ptr, this.id);
          },
          set: function(value) {
            ui.setvalue(this.form.ptr, this.id, value);
          }
        });

        Object.defineProperty(obj, "text", {
          get: function() {
            return ui.gettext(this.form.ptr, this.id);
          },
          set: function(text) {
            ui.settext(this.form.ptr, this.id, text);
          }
        });
      }
    }
  }

  f.object = function(self, id) {
    return self.objectsById[id];
  }

  f.show = function(self) {
    ui.show(self.ptr);

    if (self.first) {
      if (self.menu) {
        var pos = 1;
        for (key in self.menu) {
          var obj = self.menu[key];
          if (typeof obj == "object") {
            ui.menu(pos, pos + 1, obj.cmd, obj.text);
            if (obj.handler) {
              self.menuHandlers[pos + 1] = obj.handler;
            }
            pos = pos + 1;
          }
        }
      }
      self.first = false;
    }
  }

  f.handle = function(self, ev) {
    if (ev.type == event.ctlSelect) {
      var h = self.formHandlers[ev.control];
      if (h && typeof h == "function") {
        return h(ev, self, self.objectsById[ev.control]);
      }
    } else if (ev.type == event.menuEvent) {
      var h = self.menuHandlers[ev.item];
      if (h && typeof h == "function") {
        return h(self, ev.item);
      }
    }
    return false;
  }

  return f;
}
