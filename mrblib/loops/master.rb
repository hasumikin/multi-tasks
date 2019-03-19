# mrblib/loops/master.rb

$status = "COLD"
led = Led.new
while true
  case $status
  when "COLD"
    led.turn_off
  when "HOT"
    led.turn_on
    sleep 0.1
    led.turn_off
  end
  sleep 0.1
end
