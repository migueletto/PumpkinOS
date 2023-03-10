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
  if not def.repeating then def.repeating = false else def.repeating = true end
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
  if not def.text then def.text = "" end
  if not def.font then def.font = font.std end
  return def
end

Form.slider = function(def)
  def.kind = "slider"
  if not def.text  then def.text = "" end
  if not def.font  then def.font = font.std end
  if not def.min   then def.min  = 1 end
  if not def.max   then def.max  = 10 end
  if not def.page  then def.page = 1 end
  if not def.feedback then def.feedback = false else def.feedback = true end
  return def
end

Form.selector = function(def)
  def.kind = "selector"
  if not def.text then def.text = "" end
  if not def.font then def.font = font.std end
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

Form.list = function(def)
  def.kind = "list"
  if not def.width then def.width = 0 end
  if not def.height then def.height = 0 end
  if not def.visibleItems then def.visibleItems = 0 end
  if not def.items then def.items = { "" }  end
  if not def.font then def.font = font.std end
  return def
end

Form.popup = function(def)
  def.kind = "popup"
  if not def.visibleItems then def.visibleItems = 0 end
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

        if obj.kind == "label" then
          ui.label(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 6
          if obj.height > h then
            h = obj.height
          end
        elseif obj.kind == "button" then
          if obj.repeating then
            ui.rbutton(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          else
            ui.button(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          end
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
        elseif obj.kind == "slider" then
          if obj.feedback then
            ui.fslider(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, false, obj.min, obj.max, obj.page)
          else
            ui.slider(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font, 0, false, obj.min, obj.max, obj.page)
          end
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 6
          if obj.height > h then
            h = obj.height
          end
          if obj.handler then
            f.formHandlers[obj.id] = obj.handler
          end
        elseif obj.kind == "selector" then
          ui.selector(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.font)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 1
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
        elseif obj.kind == "list" then
          ui.list(f.ptr, obj.id, obj.x, obj.y, obj.width, obj.height, obj.visibleItems, obj.font, obj.items)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 4
          if obj.height > h then
            h = obj.height
          end
        elseif obj.kind == "popup" then
          ui.popup(f.ptr, obj.id, obj.text, obj.x, obj.y, obj.visibleItems, obj.font, obj.items)
          ui.bounds(f.ptr, obj.id, obj)
          x = x + obj.width + 4
          if obj.height > h then
            h = obj.height
          end
        end

        f.objectsById[obj.id] = obj

        obj.x = nil
        obj.y = nil
        obj.width = nil
        obj.height = nil
        obj.value = nil
        obj.text = nil

        setmetatable(obj, {
          __index = function(t, key)
            local r = nil
            if key == "value" then
              r = ui.getvalue(t.form.ptr, t.id)
            elseif key == "text" then
              r = ui.gettext(t.form.ptr, t.id)
            elseif key == "x" or key == "y" or key == "width" or key == "height" then
              local bounds = {}
              ui.bounds(t.form.ptr, t.id, bounds)
              r = bounds[key]
            end
            return r
          end,
          __newindex = function(t, key, value)
            if key == "value" then
              ui.setvalue(t.form.ptr, t.id, tonumber(value))
            elseif key == "text" then
              ui.settext(t.form.ptr, t.id, tostring(value))
            elseif key == "x" or key == "y" or key == "width" or key == "height" then
              local bounds = {}
              ui.bounds(t.form.ptr, t.id, bounds)
              bounds[key] = tonumber(value)
              ui.setbounds(t.form.ptr, t.id, bounds.x, bounds.y, bounds.width, bounds.height)
            end
          end
        })

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
        return h(ev, self, self.objectsById[ev.control])
      end
    elseif ev.type == event.menuEvent then
      local h = self.menuHandlers[ev.item]
      if h and type(h) == "function" then
        return h(self, ev.item)
      end
    end
    return false
  end

  return f
end
