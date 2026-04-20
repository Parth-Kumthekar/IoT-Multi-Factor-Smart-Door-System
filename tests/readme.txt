1. Functional Test Cases (Logic)Test IDScenarioExpected Result
TC-01
    Scenario: Authorized CardGreen LED ON
    Expected Result: Buzzer Beeps (Short), Relay Unlocks.
TC-02
    Scenario: Unauthorized CardRed LED Flashes
    Expected Result: Buzzer Beeps (Long), Relay stays Locked.
TC-03
    Scenario: Door Forced OpenIf Reed Switch opens without an NFC scan
    Expected Result: trigger "Alarm" (Red LED + Continuous Buzzer).
TC-04
    Scenario: Door Left OpenIf Reed Switch stays open for > 10 seconds after access
    Expected Result: beep a "Reminder" tone.
TC-05
    Scenario: Re-lockingOnce Reed Switch returns to "Closed" after an entry
    Expected Result: immediately turn off Green LED and engage Relay.