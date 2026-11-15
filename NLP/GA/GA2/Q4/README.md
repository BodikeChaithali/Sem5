# Plot-Twist Generator (NLP Group Assignment 2, Q4)

## Setup
pip install torch datasets pandas numpy matplotlib

## Reproduce
1. Open `plot_twist_generator.ipynb` (GPU recommended, e.g. Colab) and run all cells top to bottom (seed = 42).
2. The notebook downloads TinyStories via Hugging Face (not included in this archive).
3. Outputs are written to `outputs/`: loss curves, generation metrics, generated stories, blinded human-eval files,
   checkpoints (`rnn_lm.pt`, `lstm_lm.pt`), `vocab.json`, `settings.json`.
4. To load a checkpoint: build `LM(kind, V)` from the notebook, then `load_state_dict(torch.load(path)["state_dict"])`.

## Data
20,000 train stories (random sample of TinyStories `train`), 1,000 val + 1,000 test stories (disjoint random samples of
TinyStories `validation`). Word-level tokenizer, vocabulary 8,000 (min freq 5), stories truncated to ~100 tokens.
