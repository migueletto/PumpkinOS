Form = {}

Form.new = function(f)
  if not f.id then f.id = 1000 end
  if not f.title then f.title = "" end
  if not f.x then f.x = 0 end
  if not f.y then f.y = 0 end
  if not f.width then f.width = 160 end
  if not f.height then f.height = 160 end

  f.ptr = ui.form(f.id, f.title, f.x, f.y, f.width, f.height)

  if f.objects then
    local id = 1000
    local y = 4
    local y = 16
    for _,obj in pairs(f.objects) do
      if not obj.kind then obj.kind = "button" end
      if not obj.text then obj.text = "" end
      if not obj.x then obj.x = x end
      if not obj.y then obj.y = y end
      if not obj.id then
        obj.id = id
        id = id + 1
      end
      if obj.kind == "button" then
        ui.button(f.ptr, obj.id, obj.text, obj.x, obj.y)
      end
    end
  end

  f.show = function(self)
    ui.show(self.ptr)
  end

  return f
end
