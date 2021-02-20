quest auto_ox_schedule begin
    state start begin
		when login with game.get_event_flag("auto_ox") == 1 begin
			send_letter("Schedule OX Event")
		end
		when button or info begin
			local weekday = {"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday"}

			say_title("The OX Event Schedule:")
			say("")
			
			local OXTable = oxevent.get_auto_table()
			if table.getn(OXTable) == 0 then
				say("There isn't any info.")
				say("")
			else
				for i = 1, table.getn(OXTable), 1 do
					say(string.format("%s - %02d : %02d", weekday[OXTable[i][1] + 1], OXTable[i][2], OXTable[i][3]))
				end
				say("")
			end
		end
		when 20358.chat."Auto Ox Event" with pc.is_gm() begin
			say(string.format("State: %s", (game.get_event_flag("auto_ox") == 1) and "Active" or "Deactive"))
			say("")
			game.set_event_flag("auto_ox", select("Deactivate", "Activate") - 1)
		end
	end
end