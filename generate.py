from jinja2 import Template
from pathlib import Path

template = Template(Path('./wait-example.tpl.sv').read_text())

name_group = [
        [
            '_scclang_state_single_wait', '_scclang_wait_counter_single_wait', '_scclang_wait_next_state_single_wait',
            '_scclang_next_state_single_wait', '_scclang_next_wait_counter_single_wait', '_scclang_save_wait_next_state_single_wait'
            ],
        [
            '_scclang_state_single_wait_q', '_scclang_wait_counter_single_wait_q', '_scclang_wait_next_state_single_wait_q',
            '_scclang_state_single_wait_d', '_scclang_wait_counter_single_wait_d', '_scclang_wait_next_state_single_wait_d'
            ],
        [
            '_scclang_state_single_wait', '_scclang_wait_counter_single_wait', '_scclang_state_after_wait_single_wait',
            '_scclang_state_single_wait_next', '_scclang_wait_counter_single_wait_next', '_scclang_state_after_wait_single_wait_next'
            ],
        [
            '_scclang_single_wait_state', '_scclang_single_wait_wait_counter', '_scclang_single_wait_state_after_wait',
            '_scclang_single_wait_state_next', '_scclang_single_wait_wait_counter_next', '_scclang_single_wait_state_after_wait_next'
            ],
        ]

for idx, n in enumerate(name_group):
    a_q, b_q, c_q, a_d, b_d, c_d = n
    res = template.render(
            state=a_q, wait_counter=b_q, wait_state=c_q,
            state_next=a_d, wait_counter_next=b_d, wait_state_next=c_d,
    )
    Path('./output/wait-example-v{}.sv'.format(idx)).write_text(res)
