package tests;

import builders.BlockBuilder;
import expressions.Diff;
import expressions.Variable;
import instructions.Block;

import java.util.List;

import static org.junit.jupiter.api.Assertions.*;

class ProcedureDeclarationTest {
	@org.junit.jupiter.api.Test
	void test() {
		try {
			Block block = new BlockBuilder()
				.declareProcedure("foo1", List.of('b', 'a'), new BlockBuilder()
					.declareProcedure("foo2", List.of('b', 'a'), new BlockBuilder()
						.print(Diff.of(Variable.named('a'), Variable.named('b')))
						.build()
					)
					.build()
				)
				.build();

			int res = block.startProgram(false);

			assertEquals(res, 0);
		}
		catch (Exception e) {
			System.out.println("Exception msg: " + e.getMessage());
			assert false;
		}
	}
}
