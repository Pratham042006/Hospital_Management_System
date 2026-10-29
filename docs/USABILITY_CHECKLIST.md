# Sprint 1 usability acceptance checklist

Use this checklist during the Sprint 1 demonstration. Complete it once as an administrator,
receptionist, doctor, and billing user.

## Navigation and wording

- Only actions permitted for the signed-in role are displayed.
- `0` consistently means Back inside a submenu and Exit on the main menu.
- Patient, doctor, and appointment prompts use the same ID, date, and time wording.
- An invalid menu choice explains which displayed choices are accepted.

## Corrective messages

- Missing required patient or doctor data identifies the field to correct.
- Invalid calendar dates, past dates, and invalid times are rejected without exiting.
- Unknown IDs, duplicate contacts, unavailable doctors, and appointment conflicts have clear messages.

## Confirmations

- Patient and doctor creation display the generated ID.
- Patient, doctor, and appointment updates display success confirmation.
- Appointment cancellation confirms that history was retained.
- Exit displays a final safe-close message.

## Representative workflow

1. Administrator creates receptionist and doctor accounts.
2. Receptionist registers a patient and adds a doctor with an availability slot.
3. Receptionist schedules, reschedules, and cancels an appointment.
4. Doctor views permitted patient, doctor, and appointment information.
5. Billing user confirms that clinical/contact fields and management actions are hidden.

The automated `usability_tests` target covers the role menus, corrective validation message,
and the representative create/update/schedule/cancel workflow. The terminal wording and step count
are confirmed with this checklist.
