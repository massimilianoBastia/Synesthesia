## Synesthesia

Synesthesia is a tool for generating sinusoidal signals, blending them together, and saving the results as a raw PCM file

### Usage

```bash
./Synesthesia 1000,500,100 out.raw 2000 --verbose --logfile="log.txt"
```

### How It Works

Synesthesia is built around a **Multiple Producer-Consumer pattern** with a single shared buffer. 
Each frequency provided creates its own dedicated producer thread, so \(N\) frequencies result in \(N\) producers. 
All producer threads work together with a single consumer thread to generate and process the sine wave data.

#### Producers

1. **What They Do**:
   - Each producer generates sine wave samples for one frequency.
   - They work in chunks to avoid overwhelming memory and keep things efficient.

2. **How They Do It**:
   - Samples are generated incrementally to maintain a smooth wave, even across chunks.
   - Once a chunk is ready, it’s pushed into a shared queue, complete with frequency metadata.

3. **Why This Approach Works**:
   - Breaking data into chunks (e.g., 1000 samples at a time) keeps memory use low.
   - Incremental generation ensures that the wave stays continuous and accurate.

#### Consumer

1. **What It Does**:
   - The consumer grabs chunks of sine wave data from the queue.
   - It mixes the data, converts it to 24-bit PCM format, and writes it to the output file.

#### Queue
   - The shared queue is the middleman, ensuring safe data handoffs between producers and the consumer.
   - Producers push chunks into the queue, and the consumer pops them for processing.
   - A "done" signal lets the consumer know when all the producers are finished.

### Mixing the Sine Waves

Here’s the process:
1. The consumer pulls chunks of data from the queue.
2. It adds up the sine wave samples.
3. The result is normalized to fit within `[-1.0, 1.0]`.
4. Finally, the normalized values are converted to 24-bit PCM.

### Dealing with Floating-Point Precision

Floating-point math isn’t perfect, but it’s good enough. To keep things under control:
- **Clamping**: Ensure the values stay within `[-1.0, 1.0]` before normalization.
- **Rounding**: Map the values accurately to the PCM range using standard rounding.
