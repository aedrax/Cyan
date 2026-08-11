# Examples

Twenty runnable programs in [`examples/`](../examples/), each an API tour
that ends with a realistic "putting it together" scenario:

| Example | Shows |
|---------|-------|
| `00_primitive_types.c` | Type aliases; parsing a binary sensor packet |
| `01_option_basics.c` | Options; a config lookup chain with `and_then`/`or_else` |
| `02_result_error_handling.c` | Results; `try_ok` order-validation pipeline |
| `03_vector_collections.c` | Vectors; a sorted top-N scoreboard |
| `04_defer_cleanup.c` | Defer; a resource pyramid with early returns |
| `05_smart_pointers.c` | Unique/shared/weak; a cache with weak observers |
| `06_pattern_matching.c` | `match_*`; a connection state machine |
| `07_functional.c` | map/filter/reduce; a sensor data pipeline |
| `08_method_macros.c` | The two call styles; type sizes without vtables |
| `09_panic_handler.c` | Panics and custom handlers |
| `10_channel_communication.c` | Channels; a bounded work queue |
| `11_hashmap_dictionary.c` | Maps incl. string keys; an office phone book |
| `12_serialize_parsing.c` | S-expressions; a config round-trip |
| `13_slice_views.c` | Slices; a zero-copy tokenizer |
| `14_string_manipulation.c` | Strings; a CSV line parser |
| `15_hashset_membership.c` | HashSet; dedupe and set intersection |
| `16_bitset_integers.c` | Bitsets/uN; permissions and a saturating counter |
| `17_coro_channels.c` | **CSP**: producers/consumers under `coro_run` |
| `18_result_pipeline.c` | A realistic config-file parsing pipeline |
| `19_word_count.c` | Capstone: strings + str-map + vec sort, top-5 words |

```bash
cd examples
make        # build all
make run    # run all
```
