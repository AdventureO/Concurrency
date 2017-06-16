package com.example.conveyor;

/**
 * Created by petro on 16-Jun-17.
 */
import java.io.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

public class BlockReader implements Runnable {

    private BlockingQueue<String> blocksQueue;
    private int blocksize;
    private File filename; //file to read text from

    public BlockReader (BlockingQueue<String> blocksQueue, int blocksize, File filename) {
        this.blocksQueue = blocksQueue;
        this.blocksize = blocksize;
        this.filename = filename;
    }

    @Override
    public void run() {
        //System.out.println("BlockReader-" + Thread.currentThread().getId() + " started");
        long readingStartTime = System.nanoTime();
        readFile(filename);
        long readingExecutionTime = System.nanoTime() - readingStartTime;
        try {
            PrintWriter writer = new PrintWriter("outputFile.txt", "UTF-8");
            writer.println("Reading time: " + timeToString(readingExecutionTime));
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        //System.out.println("Reading time: " + timeToString(readingExecutionTime));
    }

    private void readFile(File filename) {
        StringBuilder sb = new StringBuilder();
        BufferedReader bufferedReader = null;
        int counter = 0;
        try {
            bufferedReader = new BufferedReader(new FileReader(filename));
            String line = bufferedReader.readLine();
            while (line != null) {
                counter += 1;
                sb.append(line);
                sb.append("\n");
                line = bufferedReader.readLine();
                if (counter == blocksize || line == null) {
                    //System.out.println("BlockReader-" + Thread.currentThread().getId() + " produced a block");
                    blocksQueue.put(sb.toString());
                    sb = new StringBuilder();
                    counter = 0;
                }
                if(line == null) {
                    //System.out.println("BlockReader-" + Thread.currentThread().getId() + " finished. Sending POISONPILL");
                    blocksQueue.put("POISONPILL");
                }
            }
        } catch (InterruptedException | IOException e) {
            e.printStackTrace();
        } finally {
            try {
                bufferedReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private String timeToString(long time) {
        return String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
    }
}