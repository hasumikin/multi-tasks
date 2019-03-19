thermistor = Thermistor.new

while true
  $status = if thermistor.temperature > 30
    "HOT"
  else
    "COLD"
  end
  sleep 1
end

