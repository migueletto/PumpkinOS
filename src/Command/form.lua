-- Form class

Form = {}

Form.newline = "newline"

Form.menu = function(def)
  if not def.cmd  then def.cmd = 0 end
  if not def.text then def.text = "" end
  return def
end

Form.label = function(def)
  def.kind = "label"
  if not def.text then def.text = "" end
  if not def.font then def.font = font.bold end
  return def
end

Form.button = function(def)
  def.kind = "button"
  if not def.text then def.text = "" end
  if not def.font then def.font = font.std end
  return def
end

Form.pushButton = function(def)
  def.kind = "pushButton"
  if not def.group then def.group = 1 end
  if not def.text  then def.text = "" end
  if not def.font  then def.font = font.std end
  return def
end

Form.checkbox = function(def)
  def.kind = "checkbox"
  if not def.text  then def.text = "" end
  if not def.font  then def.font = font.std end
  return def
end

Form.field = function(def)
  def.kind = "field"
  if not def.cols then def.cols = 16 end
  if not def.rows then def.rows = 1 end
  if not def.max  then def.max = def.rows * def.cols end
  if not def.font then def.font = font.std end
  return def
end

Form.new = function(f)
  if not f.id then f.id = 1000 end
  if not f.title then f.title = "" end
  if not f.x then f.x = 0 end
  if not f.y then f.y = 0 end
  if not f.width then f.width = 160 end
  if not f.height then f.height = 160 end

  f.first = true
  f.ptr = ui.form(f.id, f.title, f.x, f.y, f.width, f.height)

  f.menuHandlers = {}
  f.formHandlers = {}
  f.objectsById  = {}

  if not f.about then
    f.about = "Application created with Command."
  end
  f.menuHandlers[1] = function()
    ui.about(f.about)
  end

  if f.objects then
    local id = 1000
    local x = 4
    local y = 20
    local h = 0
    for _,obj in pairs(f.objects) do
      if type(obj) == "string" then
        if obj == Form.newline then
          x = 4
          y = y + h + 6
          h = 0
        end
      elseif type(obj) == "table" then
        if not obj.x then obj.x = x end
        if not obj.y then obj.y = y end
        if not obj.id then
          obj.id = id
          id = id + 1
        else
          id = obj.id + 1
        end

        obj.form = f

        obj.label = function(self)
          return ui.getlabel(self.form.ptr, self.id)
        end

        obj.value = function(self)
          return ui.getvalue(self.form.ptr, self.id)
        end

        f.objectsById[obj.id] = obj

        if obj.kind == "label" then
          ui.label(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 6
          if obj.height > h then
            h = obj.height
          end
        elseif obj.kind == "button" then
          ui.button(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 6
          if obj.height > h then
            h = obj.height
          end
          if obj.handler then
            f.formHandlers[obj.id] = obj.handler
          end
        elseif obj.kind == "pushButton" then
          ui.pushbutton(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, obj.group, obj.selected)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 1
          if obj.height > h then
            h = obj.height
          end
          if obj.handler then
            f.formHandlers[obj.id] = obj.handler
          end
        elseif obj.kind == "checkbox" then
          ui.checkbox(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, obj.selected)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 6
          if obj.height > h then
            h = obj.height
          end
          if obj.handler then
            f.formHandlers[obj.id] = obj.handler
          end
        elseif obj.kind == "field" then
          ui.field(f.ptr, obj.id, obj.x, obj.y, obj.cols, obj.rows, obj.max, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 4
          if obj.height > h then
            h = obj.height
          end
        end
      end
    end
  end

  f.object = function(self, id)
    return self.objectsById[id]
  end

  f.show = function(self)
    ui.show(self.ptr)

    if self.first then
      if self.menu then
        local pos = 1
        for _,obj in pairs(self.menu) do
          if type(obj) == "table" then
            ui.menu(pos, pos + 1, obj.cmd, obj.text)
            if obj.handler then
              self.menuHandlers[pos + 1] = obj.handler
            end
            pos = pos + 1
          end
        end
      end
      self.first = false
    end
  end

  f.handle = function(self, ev)
    if ev.type == event.ctlSelect then
      local h = self.formHandlers[ev.control]
      if h and type(h) == "function" then
        local label = ui.getlabel(self.ptr, ev.control)
        if label then
          ev.label = label
        end
        return h(ev, self, self.objectsById[ev.control])
      end
    elseif ev.type == event.menuEvent then
      local h = self.menuHandlers[ev.item]
      if h and type(h) == "function" then
        return h(self)
      end
    end
    return false
  end

  return f
end
