package com.karabyn;

import java.io.*;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;

/**
 * Created by petro on 18-Apr-17.
 */

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
        System.out.println("Reading time: " + timeToString(readingExecutionTime));
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
                    blocksQueue.put(sb.toString());
                    sb = new StringBuilder();
                    counter = 0;
                }
                if(line == null) {
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
        String timeString = String.format("%d s %d ms", TimeUnit.NANOSECONDS.toSeconds(time),
                TimeUnit.NANOSECONDS.toMillis(time) - TimeUnit.SECONDS.toMillis(TimeUnit.NANOSECONDS.toSeconds(time)));
        return timeString;
    }
}