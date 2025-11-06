import requests
import time

ship_id = 'player_pawn'

print('Getting initial position...')
r1 = requests.get(f'http://localhost:8080/get_position/{ship_id}')
initial = r1.json()
print(f'Initial: {initial}')

print('\nApplying thrust X+1.0 for 3 seconds...')
for i in range(3):
    requests.post('http://localhost:8080/set_input', json={
        'ship_id': ship_id,
        'thrust': {'x': 1.0, 'y': 0.0, 'z': 0.0},
        'rotation': {'pitch': 0.0, 'yaw': 0.0, 'roll': 0.0}
    })
    time.sleep(1)
    print(f'  Tick {i+1}')

print('\nGetting final position...')
r2 = requests.get(f'http://localhost:8080/get_position/{ship_id}')
final = r2.json()
print(f'Final: {final}')

if initial.get('success') and final.get('success'):
    initial_x = initial['data']['position']['x']
    final_x = final['data']['position']['x']
    distance = abs(final_x - initial_x)
    print(f'\n{"="*70}')
    if distance > 10:
        print('[SUCCESS] SHIP MOVED!')
        print(f'Distance traveled: {distance:.2f} units')
    else:
        print('[FAIL] Ship did not move significantly')
        print(f'Distance: {distance:.2f} units')
    print("="*70)
