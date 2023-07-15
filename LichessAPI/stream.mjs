
export class NdjsonStream {

	streamReader;
	buffer = [];
	done = false;

	constructor(response) {
		this.streamReader = response.body.getReader();
		this.streamReader.closed.then(() => this.done = true);
	}

	async forEach(action) {
		let json;
		do {
			json = await this.read();
			action(json);
		} while (json != undefined);
	}

	async read() {
		while (this.buffer.length == 0 && !this.done) {
			let streamValue = (await this.streamReader.read()).value;
			let text = new TextDecoder().decode(streamValue);
			if (text.trim().length == 0) continue;
	
			for (let textElement of text.split("\n")) {
				if (textElement.trim().length == 0) continue;
				try {
					let json = JSON.parse(textElement);
					this.buffer.push(json);
				} catch (error) {
					console.error(eventText, error);
				}
			}
		}

		if (this.buffer.length > 0) return this.buffer.shift();
	}

}
