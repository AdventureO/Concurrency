def cleanWord(word):
    word = ''.join(x.lower() for x in word if x.isalnum())
    return word;

if __name__ == '__main__':
	print('Word cleaning for (samle) concurrent word counters')
	