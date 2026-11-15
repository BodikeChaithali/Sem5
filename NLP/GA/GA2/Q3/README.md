# Abstractive Text Summarization with a Transformer Encoder–Decoder

**Course:** DSL 504 — Natural Language Processing  
**Assignment:** NLP Group Assignment 2 — Question 3  
**Framework:** PyTorch  
**Dataset:** CNN/DailyMail 3.0.0  
**Task:** Generate abstractive summaries of English news articles.

## 1. Project Overview

This project implements a complete Transformer encoder–decoder for abstractive news summarization using PyTorch. The model reads a news article and generates a summary autoregressively, one token at a time.

The core Transformer components are implemented from scratch using basic PyTorch operations rather than prebuilt Transformer or multi-head attention modules.

The implementation includes:

- Multi-head scaled dot-product attention
- Sinusoidal positional encoding
- Position-wise feed-forward networks
- Transformer encoder and decoder layers
- Encoder–decoder cross-attention
- Residual connections and layer normalization
- Source padding masks and causal decoder masking
- Teacher-forcing training and padding-aware cross-entropy loss
- Greedy autoregressive decoding
- ROUGE evaluation, qualitative error analysis, and attention visualization

The pretrained `bert-base-uncased` tokenizer is used for tokenization only. The Transformer model parameters are trained from scratch; no pretrained summarization-model weights are used.

## 2. Dataset

The model uses the CNN/DailyMail dataset, version 3.0.0, loaded through Hugging Face Datasets.

Each example contains:
- `article`: The source news article
- `highlights`: The reference summary
- `id`: The example identifier

### Dataset Splits

| Split | Available examples | Used in the experiment |
|---|---:|---:|
| Training | 287,113 | 60,000 |
| Validation | 13,368 | 13,368 |
| Test | 11,490 | 11,490 |

A training subset of 60,000 randomly selected examples is used to keep the experiment computationally manageable. Random seed 42 is used for reproducibility.

The validation split is used for checkpoint selection and monitoring training. Final ROUGE evaluation is performed on the full test split.

**Dataset source:** [CNN/DailyMail on Hugging Face](https://huggingface.co/datasets/abisee/cnn_dailymail)

The dataset is downloaded automatically when the notebook runs. Do not include the dataset in the submission archive.

## 3. Preprocessing and Tokenization

- **Tokenizer:** `bert-base-uncased` WordPiece tokenizer
- **Vocabulary size:** 30,522 tokens
- **Text preprocessing:** Whitespace normalization and lowercasing through the uncased tokenizer
- **Maximum article length:** 400 tokens
- **Maximum summary length:** 128 tokens, including special tokens
- **Start token:** `[CLS]`
- **End token:** `[SEP]`
- **Padding token:** `[PAD]`
- **Summary sentence boundaries:** `[unused1]` is used to preserve sentence boundaries for ROUGE-Lsum evaluation

During training, the target summary is shifted right to form the decoder input, while the prediction target is shifted left. Padding positions are excluded from the cross-entropy loss.

A length analysis of 2,000 validation examples found that 78.2% of articles exceeded the 400-token input limit. This truncation can remove useful context and is an important limitation of the experiment.

## 4. Model Architecture

The encoder and decoder are implemented using custom PyTorch modules.

| Hyperparameter | Configuration |
|---|---:|
| Encoder layers | 3 |
| Decoder layers | 3 |
| Model dimension (`d_model`) | 256 |
| Attention heads | 4 |
| Feed-forward dimension (`d_ff`) | 1,024 |
| Dropout | 0.1 |
| Trainable parameters | 13,343,232 |
| Maximum article length | 400 tokens |
| Maximum summary length | 128 tokens |

The implementation includes:

- Encoder self-attention
- Masked decoder self-attention
- Encoder–decoder cross-attention
- Sinusoidal positional encodings
- Position-wise feed-forward networks
- Residual connections and layer normalization
- Tied token embedding and output projection weights

The attention mechanism uses explicit query, key, and value projections, scaled attention scores, masking, softmax, and weighted value aggregation.

The model does not use `nn.Transformer`, `nn.TransformerEncoder`, `nn.TransformerDecoder`, `nn.MultiheadAttention`, `torch.nn.functional.scaled_dot_product_attention`, or a pretrained summarization model.

The notebook includes sanity checks for causal masking, padding masking, attention probability distributions, and output tensor shapes.

## 5. Training Configuration

| Setting | Value |
|---|---|
| Optimizer | Adam |
| Adam betas | (0.9, 0.98) |
| Adam epsilon | `1e-9` |
| Learning-rate schedule | Noam warm-up and inverse-square-root decay |
| Warm-up steps | 2,000 |
| Schedule factor | 0.5 |
| Batch size | 32 |
| Maximum epochs | 12 |
| Early-stopping patience | 3 validation checks/epochs without improvement |
| Gradient clipping | Maximum norm 1.0 |
| Loss | Cross-entropy, ignoring `[PAD]` tokens |
| Mixed precision | Disabled for the recorded run |
| Random seed | 42 |

The model is trained with teacher forcing. During inference, greedy decoding generates tokens sequentially until `[SEP]` is produced or the maximum target length is reached. The encoder output is computed once for each article and reused during decoding.

The best checkpoint is selected using validation loss.

## 6. How to Run

### Recommended: Google Colab

1. Upload `Q3_transformer_summary.ipynb` to Google Colab.
2. Select **Runtime → Change runtime type** and choose a GPU runtime, such as a T4 GPU if available.
3. Run the notebook cells in order, starting with the dependency-installation and setup cells.
4. Allow the required dataset and tokenizer resources to download.
5. Run preprocessing, model checks, training, validation, checkpoint selection, test generation, ROUGE evaluation, qualitative analysis, and attention visualization in sequence.
6. Save the notebook with its outputs after execution.

A GPU is strongly recommended because model training and full test-set generation are computationally intensive.

### Required Libraries

The notebook uses the following Python packages:

```bash
pip install datasets transformers rouge_score nltk matplotlib pandas tqdm
```

PyTorch and NumPy are also required. Google Colab typically provides them in its standard runtime. If running locally, install a PyTorch build compatible with your Python version and hardware.

The notebook may use a checkpoint path configured for Google Colab. Update the path if running in a different environment.

**Reproducibility:** Random seeds are set in the notebook, but exact results may vary across hardware, software versions, and device kernels.

## 7. Results

The model is evaluated on the complete test split of 11,490 examples using ROUGE F1 scores multiplied by 100, with stemming enabled.

| System | ROUGE-1 | ROUGE-2 | ROUGE-L | ROUGE-Lsum |
|---|---:|---:|---:|---:|
| Transformer (greedy decoding) | 27.10 | 6.79 | 19.08 | 25.20 |
| Lead-3 baseline | 40.05 | 17.48 | 25.02 | 36.35 |

The best validation loss recorded was **3.7040 at epoch 12**.

The Lead-3 baseline selects the first three sentences of each article and does not require a trained summarization model. It outperforms the implemented Transformer on all four ROUGE metrics in this experiment.

These results are reported as measured. They indicate that the current from-scratch model does not outperform the extractive baseline under the selected training configuration and computational budget.

## 8. Evaluation and Visualization

The notebook contains:

- Training and validation loss and perplexity curves
- Learning-rate schedule visualization
- Generated summaries compared with reference summaries
- Qualitative analysis of at least five test examples
- Repetition and unsupported-number diagnostics
- ROUGE-1, ROUGE-2, ROUGE-L, and ROUGE-Lsum evaluation on the full test set
- Encoder–decoder cross-attention heatmaps from the final decoder layer during greedy generation
- Attention visualizations averaged across four heads and displayed separately for each head

The attention heatmaps help inspect which source tokens receive attention while generating summary tokens. They do not, by themselves, establish that a generated statement is factually correct.

## 9. Limitations

The main limitations observed in this experiment are:

1. **Limited training subset:** Only 60,000 of the 287,113 available training examples are used.
2. **Input truncation:** Articles are restricted to 400 tokens, potentially excluding relevant information.
3. **Training from scratch:** The Transformer does not use pretrained language-model weights.
4. **Repetition:** Generated summaries sometimes contain repeated phrases or token loops.
5. **Factual errors:** Some outputs contain incorrect names, numbers, or unsupported claims.
6. **Greedy decoding:** Beam search and explicit repetition-control methods are not used.
7. **Evaluation limitations:** ROUGE measures textual overlap and does not fully capture factual consistency, readability, or usefulness.
8. **Reproducibility:** The reported results come from one recorded experimental run.

Possible future improvements include increasing the training subset, improving long-article handling, experimenting with model configurations, and evaluating alternative decoding strategies using validation data.

## 10. Folder Structure:

```text
Q3-Transformer-Summarization/
├── README.md
├── Q3_transformer_summary.ipynb
└── Q3_Abstractive_Text_Summarization_Report.pdf
```
